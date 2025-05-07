#include "Session.h"
#include "Server.h"
#include <memory>
#include <functional>
#include <mutex>

void Session::start()
{
	memset(m_data, 0, max_length);

	//调用异步读回调，一旦收到数据就会触发Session::handle_read函数
	//这里必须使用shared_from_this()来实现引用计数同步，而Session必须继承enable_shared_from_this
	m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

std::string& Session::uuid()
{
	return m_uuid;
}

void Session::send(char* msg, int max_length)
{
	bool pending = false;

	//加锁保护
	std::lock_guard<std::mutex> guard(m_send_queue_lock);

	if(m_send_queue.size() > 0)
	{
		pending = true;   //说明前面的数据还没发送完
	}

	m_send_queue.push(std::make_shared<MsgNode>(msg, max_length));

	if (pending)
	{
		//前面的数据还没发送完则直接返回，因为写完成回调函数里会判断发送队列是否还有新的数据要发，如果有就会继续发
		//同时也会继续调用异步发送函数并注册写完成回调函数
		return;   
	}

	//如果此时前面的数据已经都发完了，则需要调用异步发送函数并注册回调函数
	boost::asio::async_write(m_socket, boost::asio::buffer(msg, max_length), std::bind(&Session::handle_write, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void Session::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session)
{
	if (ec)
	{
		std::cout << "handle read error" << std::endl;

		//改用了智能指针，不能再手动释放了，改为将智能指针从Server的数据结构中移除
		m_server->clear_session(m_uuid);

		return;
	}

	//没有发生错误
	std::cout << "received:" << m_data << std::endl;

	//发送数据
	send(m_data, bytes_transferred);

	//再次调用async_read_some()监听数据
	memset(m_data, 0, max_length);
	//m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, std::placeholders::_1, std::placeholders::_2, self_session));
}

void Session::handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session)
{
	if (ec)
	{
		std::cout << "handle write error" << std::endl;

		//改用了智能指针，不能再手动释放了，改为将智能指针从Server的数据结构中移除
		m_server->clear_session(m_uuid);

		return;
	}

	//加锁保护
	std::lock_guard<std::mutex> guard(m_send_queue_lock);

	//写回调函数触发则说明一定全部发了，则将发送队列中的第一个MsgNode移除
	m_send_queue.pop();

	if (!m_send_queue.empty())
	{
		//发送队列中还有其他的消息需要发送，则继续发送
		auto& node = m_send_queue.front();

		boost::asio::async_write(m_socket, boost::asio::buffer(node->m_data, node->m_max_len), std::bind(&Session::handle_write, this, std::placeholders::_1, std::placeholders::_2, self_session));
	}
}