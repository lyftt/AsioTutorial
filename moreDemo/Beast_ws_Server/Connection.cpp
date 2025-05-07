#include "Connection.h"
#include "ConnectionMgr.h"

Connection::Connection(net::io_context& ioc):m_ioc(ioc), m_ws_ptr(std::make_unique<beast::websocket::stream<beast::tcp_stream>>(boost::asio::make_strand(ioc)))
{
	//生产uuid
	boost::uuids::random_generator generator;

	auto uuid = generator();   //生成
	m_uuid = boost::uuids::to_string(uuid);   //转字符串
}

std::string Connection::uuid()
{
	return m_uuid;
}

net::ip::tcp::socket& Connection::socket()
{
	return boost::beast::get_lowest_layer(*m_ws_ptr).socket();  //最底层是tcp
}

void Connection::async_accept()
{
	auto self = shared_from_this();

	//self是拷贝捕获，需要延长生命周期
	m_ws_ptr->async_accept([self](boost::system::error_code ec) {
		try
		{
			if (!ec)   //连接没发生错误
			{
				ConnectionMgr::get_instance().add_connection(self);   //加入连接管理类中
				self->start();   //启动数据收发
			}
			else
			{
				std::cout << "websocket async accept failed, error is " << ec.what() << std::endl;
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "websocket async accept exception is " << e.what() << std::endl;
		}
	});
}

void Connection::start()
{
	auto self = shared_from_this();

	m_ws_ptr->async_read(m_recv_buffer, [self](boost::system::error_code ec, std::size_t buffer_bytes) {
		try
		{
			if (ec)
			{
				std::cout << "async read failed:" << ec.what() << std::endl;
				ConnectionMgr::get_instance().remove_connection(self->uuid());
				return;
			}

			//正常情况，读取到了数据
			self->m_ws_ptr->text(self->m_ws_ptr->got_text());   //设置文本类型，got_text会判断收到的数据是否是文本类型，text函数则是设置文本类型
			std::string recv_data = boost::beast::buffers_to_string(self->m_recv_buffer.data());   //buffer数据转成string
			self->m_recv_buffer.consume(self->m_recv_buffer.size());       //清空
			std::cout << "websocket recv data:" << recv_data << std::endl; //打印收到的数据

			self->async_send(std::move(recv_data));   //发送数据给对端

			self->start();   //继续监听收到的数据
		}
		catch (const std::exception& e)
		{
			std::cout << "async read exception:" << e.what() << std::endl;
			ConnectionMgr::get_instance().remove_connection(self->uuid());
		}
	});
}

void Connection::async_send(const std::string& msg)
{
	{  //锁的范围(RAII)
		std::lock_guard<std::mutex> guard(m_send_lock);

		auto len = m_send_queue.size();
		m_send_queue.push(msg);

		if (len > 0)   //之前就有数据还未发送则可以直接返回
		{
			return;
		}
	}

	//发送
	auto self = shared_from_this();
	m_ws_ptr->async_write(boost::asio::buffer(msg.c_str(), msg.size()), [self](boost::system::error_code ec, std::size_t nsize) {
		try
		{
			if (ec)   //出错
			{
				std::cout << "async send failed:" << ec.what() << std::endl;
				ConnectionMgr::get_instance().remove_connection(self->uuid());
				return;
			}

			std::string send_msg;

			{
				std::lock_guard<std::mutex> guard(self->m_send_lock);
				self->m_send_queue.pop();   //发送成功就需要弹出

				if (self->m_send_queue.empty())
				{
					return;
				}

				send_msg = self->m_send_queue.front();   //拷贝出队首元素
			}

			self->async_send(std::move(send_msg));
		}
		catch (const std::exception& e)
		{
			std::cout << "async write exception:" << e.what() << std::endl;
			ConnectionMgr::get_instance().remove_connection(self->uuid());
		}
	});
}


