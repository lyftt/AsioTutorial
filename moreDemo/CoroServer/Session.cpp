#include "Session.h"
#include "boost/uuid/uuid_io.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/asio/co_spawn.hpp"    //Э��
#include "boost/asio/detached.hpp"    //Э��
#include "boost/asio.hpp"
#include "const.h"
#include <memory>
#include <iostream>
#include "Server.h"
#include "LogicSystem.h"

Session::Session(boost::asio::io_context& ioc, Server* svr):m_ioc(ioc), m_server(svr),m_socket(ioc),m_close(false)
{
	//����ΨһID
	boost::uuids::uuid a = boost::uuids::random_generator()();
	m_uuid = boost::uuids::to_string(a);

	//������Ϣͷ���ڵ��ʼ��
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

	//����Э�̵����ݽ���
	//����һ���ɵ��ö����ܱ�Э��ʹ�ã��Ƿ������ͱ�����boost::asio::awaitable<T>
	boost::asio::co_spawn(m_ioc, [=]()->boost::asio::awaitable<void> {
		try
		{
			//��Ҫ�ж��Ƿ�ر���
			for (; !m_close;)
			{
				m_recv_head_node->clear();

				//�ȴ�ͬ����ȡ��ɣ����첽�����ͬ�����ó�����
				std::size_t n = co_await boost::asio::async_read(m_socket, 
					boost::asio::buffer(m_recv_head_node->m_data, m_recv_head_node->m_total_len),
					boost::asio::use_awaitable);

				//�Զ˹ر�
				if (n == 0)
				{
					std::cout << "peer closed" << std::endl;
					close();
					m_server->clear_session(m_uuid);

					//Э�̷��أ�����
					co_return;
				}

				//����ͷ����Ϣ
				short msg_id = 0;
				short msg_len = 0;

				//�ֽ���ת��
				memcpy(&msg_id, m_recv_head_node->m_data, HEAD_ID_LEN);
				msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
				memcpy(&msg_len, m_recv_head_node->m_data + HEAD_ID_LEN, HEAD_DATA_LEN);
				msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
				std::cout << "msg_id:" << msg_id << ", msg_len:" << msg_len << std::endl;

				//��Щid�����жϺͳ����ж�
				if (msg_len > MAX_LENGTH)
				{
					std::cout << "msg_len invalid" << std::endl;
					close();

					m_server->clear_session(m_uuid);

					//Э�̷��أ�����
					co_return;
				}
				
				//��ʼ����Ϣ����
				m_recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);
				n = co_await boost::asio::async_read(m_socket, 
					boost::asio::buffer(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len), 
					boost::asio::use_awaitable);

				//�Զ˹ر�
				if (0 == n)
				{
					std::cout << "peer closed" << std::endl;
					close();
					m_server->clear_session(m_uuid);

					//Э�̷��أ�����
					co_return;
				}

				std::cout << "recv data:" << m_recv_msg_node->m_data << std::endl;

				//Ͷ�ݸ��߼��̴߳���
				LogicSystem::get_instance().post_msg_to_queue(std::make_shared<LogicNode>(shared_from_this(), m_recv_msg_node));
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "error:" << std::endl;

			//�ر�
			close();

			//�����������Session��¼
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
* ���Ͳ��ʺ���Э��:
*��1����Ϊ���Ϳ������߼�Э����,
*��2��Э��Ҳ�ǻ�������Դ�ģ����κ�ʱ�̶����ܷ�����������ʹ����ܶ࣬��ô������ܶ�Э��
*/
void Session::send(const char* msg, short msg_len, short msg_id)
{
	//����
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
	lock.unlock();   //����ȡ���������

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
			m_send_queue.pop();   //д��ɻص���ô�������һ��

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
