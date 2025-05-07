#include "Session.h"
#include "boost/uuid/uuid_io.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/asio/co_spawn.hpp"    //协程
#include "boost/asio/detached.hpp"    //协程
#include "boost/asio.hpp"
#include "const.h"
#include <memory>
#include <iostream>
#include "Server.h"
#include "LogicSystem.h"

Session::Session(boost::asio::io_context& ioc, Server* svr):m_ioc(ioc), m_server(svr),m_socket(ioc),m_close(false)
{
	//计算唯一ID
	boost::uuids::uuid a = boost::uuids::random_generator()();
	m_uuid = boost::uuids::to_string(a);

	//接收消息头部节点初始化
	m_recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

Session::~Session()
{
	try
	{
		std::cout << "Session " << m_uuid << " destruct" << std::endl;
		close();
	}
	catch (const std::exception& e)
	{
		std::cout << "error:" << e.what() << std::endl;
	}
}

boost::asio::ip::tcp::socket& Session::socket()
{
	return m_socket;
}

std::string& Session::uuid()
{
	return m_uuid;
}

void Session::start()
{
	auto session_this = shared_from_this();

	//开启协程的数据接收
	//想让一个可调用对象能被协程使用，那返回类型必须是boost::asio::awaitable<T>
	boost::asio::co_spawn(m_ioc, [=]()->boost::asio::awaitable<void> {
		try
		{
			//需要判断是否关闭了
			for (; !m_close;)
			{
				m_recv_head_node->clear();

				//等待同步读取完成，将异步变成了同步，让出调度
				std::size_t n = co_await boost::asio::async_read(m_socket, 
					boost::asio::buffer(m_recv_head_node->m_data, m_recv_head_node->m_total_len),
					boost::asio::use_awaitable);

				//对端关闭
				if (n == 0)
				{
					std::cout << "peer closed" << std::endl;
					close();
					m_server->clear_session(m_uuid);

					//协程返回，结束
					co_return;
				}

				//解析头部信息
				short msg_id = 0;
				short msg_len = 0;

				//字节序转换
				memcpy(&msg_id, m_recv_head_node->m_data, HEAD_ID_LEN);
				msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
				memcpy(&msg_len, m_recv_head_node->m_data + HEAD_ID_LEN, HEAD_DATA_LEN);
				msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
				std::cout << "msg_id:" << msg_id << ", msg_len:" << msg_len << std::endl;

				//做些id类型判断和长度判断
				if (msg_len > MAX_LENGTH)
				{
					std::cout << "msg_len invalid" << std::endl;
					close();

					m_server->clear_session(m_uuid);

					//协程返回，结束
					co_return;
				}
				
				//开始读消息内容
				m_recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);
				n = co_await boost::asio::async_read(m_socket, 
					boost::asio::buffer(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len), 
					boost::asio::use_awaitable);

				//对端关闭
				if (0 == n)
				{
					std::cout << "peer closed" << std::endl;
					close();
					m_server->clear_session(m_uuid);

					//协程返回，结束
					co_return;
				}

				std::cout << "recv data:" << m_recv_msg_node->m_data << std::endl;

				//投递给逻辑线程处理
				LogicSystem::get_instance().post_msg_to_queue(std::make_shared<LogicNode>(shared_from_this(), m_recv_msg_node));
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "error:" << std::endl;

			//关闭
			close();

			//清除服务器的Session记录
			m_server->clear_session(m_uuid);
		}
	}, 
	boost::asio::detached);
}

void Session::close()
{
	m_socket.close();
	m_close = true;
}

/*
* 发送不适合用协程:
*（1）因为发送可能在逻辑协程里,
*（2）协程也是会消耗资源的，且任何时刻都可能发生，如果发送次数很多，那么会产生很多协程
*/
void Session::send(const char* msg, short msg_len, short msg_id)
{
	//加锁
	std::unique_lock<std::mutex> lock(m_send_lock);

	int send_que_size = m_send_queue.size();
	if (send_que_size > MAX_SEND_QUEUE)
	{
		std::cout << "Session " << m_uuid << " send queue full, size is " << MAX_SEND_QUEUE << std::endl;
		return;
	}

	m_send_queue.push(std::make_shared<SendNode>(msg, msg_len, msg_id));

	if (send_que_size > 0)
	{
		return;
	}

	auto node = m_send_queue.front();
	lock.unlock();   //数据取出立马解锁

	boost::asio::async_write(m_socket, boost::asio::buffer(node->m_data, node->m_total_len), std::bind(&Session::handle_write, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void Session::send(const std::string& msg, short msg_id)
{
	send(msg.c_str(), msg.length(), msg_id);
}

void Session::handle_write(boost::system::error_code ec, std::size_t bytes_transferred, std::shared_ptr<Session> session)
{
	try
	{
		if (!ec)
		{
			std::unique_lock<std::mutex> lock(m_send_lock);
			m_send_queue.pop();   //写完成回调那么必须出队一个

			if (!m_send_queue.empty())
			{
				auto node = m_send_queue.front();
				lock.unlock();

				boost::asio::async_write(m_socket, boost::asio::buffer(node->m_data, node->m_total_len), std::bind(&Session::handle_write, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
			}
		}
		else
		{
			std::cout << "handle write error:" << ec.what() << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "handle write exception:" << e.what() << std::endl;
		close();

		m_server->clear_session(m_uuid);
	}
}
