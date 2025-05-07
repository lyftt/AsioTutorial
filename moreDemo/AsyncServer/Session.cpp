#include "Session.h"
#include <memory>
#include <functional>

void Session::start()
{
	memset(m_data, 0, max_length);

	//调用异步读回调，一旦收到数据就会触发Session::handle_read函数
	m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2));
}

void Session::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if (ec)
	{
		std::cout << "handle read error" << std::endl;

		//释放掉Session自己，因为对方已经不在了，但存在隐患
		delete this;

		return;
	}

	//没有发生错误
	std::cout << "received:" << m_data << std::endl;

	//隐患，如果这么写，那么如果client在调用async_write之前退出，则会导致handle_read和handle_write都被触发，都会发现ec是存在错误的，导致delete2次
	//其实不仅如此，假设这两个回调函数中的一个先被调用了，把this释放了，那么此时Session已经不存在了，内存已经无效了，之后的第二次调用回调函数里如果
	//存在任何操作成员变量的操作都会引发崩溃！
	//所以Session的声明周期是一个问题
	//m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2));

	//发送回显数据
	//m_socket.async_send(boost::asio::buffer(m_data, bytes_transferred), std::bind(Session::handle_write, this, std::placeholders::_1));
	boost::asio::async_write(m_socket, boost::asio::buffer(m_data, bytes_transferred), std::bind(&Session::handle_write, this, std::placeholders::_1, std::placeholders::_2));
}

void Session::handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if (ec)
	{
		std::cout << "handle write error" << std::endl;

		//释放掉Session自己，因为对方已经不在了，但存在隐患
		delete this;

		return;
	}

	//没有发生错误
	memset(m_data, 0, max_length);

	//继续监听读数据
	m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2));
}

Server::Server(boost::asio::io_context& ioc, short port):m_ioc(ioc),m_ac(m_ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	start_accept();
}

void Server::start_accept()
{
	//开始监听
	Session* session = new Session(m_ioc);

	//回调函数只能有一个参数，asio定义的，所以这里需要绑定下第一个参数
	m_ac.async_accept(session->socket(), std::bind(&Server::handle_accept, this, session, std::placeholders::_1));
}

void Server::handle_accept(Session* session, const boost::system::error_code& ec)
{
	if (ec)
	{
		std::cout << "handle accept error" << std::endl;

		//释放掉Session
		delete this;
	}
	else
	{
		//开启session会话，收发数据
		session->start();
	}

	//处理完新的连接后，继续接收新的连接
	start_accept();
}
