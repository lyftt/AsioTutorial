#include "Connection.h"
#include "ConnectionMgr.h"

Connection::Connection(net::io_context& ioc):m_ioc(ioc), m_ws_ptr(std::make_unique<beast::websocket::stream<beast::tcp_stream>>(boost::asio::make_strand(ioc)))
{
	//����uuid
	boost::uuids::random_generator generator;

	auto uuid = generator();   //����
	m_uuid = boost::uuids::to_string(uuid);   //ת�ַ���
}

std::string Connection::uuid()
{
	return m_uuid;
}

net::ip::tcp::socket& Connection::socket()
{
	return boost::beast::get_lowest_layer(*m_ws_ptr).socket();  //��ײ���tcp
}

void Connection::async_accept()
{
	auto self = shared_from_this();

	//self�ǿ���������Ҫ�ӳ���������
	m_ws_ptr->async_accept([self](boost::system::error_code ec) {
		try
		{
			if (!ec)   //����û��������
			{
				ConnectionMgr::get_instance().add_connection(self);   //�������ӹ�������
				self->start();   //���������շ�
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

			//�����������ȡ��������
			self->m_ws_ptr->text(self->m_ws_ptr->got_text());   //�����ı����ͣ�got_text���ж��յ��������Ƿ����ı����ͣ�text�������������ı�����
			std::string recv_data = boost::beast::buffers_to_string(self->m_recv_buffer.data());   //buffer����ת��string
			self->m_recv_buffer.consume(self->m_recv_buffer.size());       //���
			std::cout << "websocket recv data:" << recv_data << std::endl; //��ӡ�յ�������

			self->async_send(std::move(recv_data));   //�������ݸ��Զ�

			self->start();   //���������յ�������
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
	{  //���ķ�Χ(RAII)
		std::lock_guard<std::mutex> guard(m_send_lock);

		auto len = m_send_queue.size();
		m_send_queue.push(msg);

		if (len > 0)   //֮ǰ�������ݻ�δ���������ֱ�ӷ���
		{
			return;
		}
	}

	//����
	auto self = shared_from_this();
	m_ws_ptr->async_write(boost::asio::buffer(msg.c_str(), msg.size()), [self](boost::system::error_code ec, std::size_t nsize) {
		try
		{
			if (ec)   //����
			{
				std::cout << "async send failed:" << ec.what() << std::endl;
				ConnectionMgr::get_instance().remove_connection(self->uuid());
				return;
			}

			std::string send_msg;

			{
				std::lock_guard<std::mutex> guard(self->m_send_lock);
				self->m_send_queue.pop();   //���ͳɹ�����Ҫ����

				if (self->m_send_queue.empty())
				{
					return;
				}

				send_msg = self->m_send_queue.front();   //����������Ԫ��
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


