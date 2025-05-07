#include "Server.h"
#include "Session.h"
#include "IOServicePool.h"

Server::Server(boost::asio::io_context& ioc, short port) :m_ioc(ioc), m_ac(m_ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	start_accept();
}

void Server::start_accept()
{
	//这里使用IOServicePool中的io_context
	std::shared_ptr<Session> session = std::make_shared<Session>(IOServicePool::get_instance()->get_io_service(), this);

	m_ac.async_accept(session->socket(), std::bind(&Server::handle_accept, this, session, std::placeholders::_1));
}

void Server::handle_accept(std::shared_ptr<Session> session, const boost::system::error_code& ec)
{
	if (ec)
	{
		std::cout << "handle accept error" << std::endl;
	}
	else
	{
		//开启session会话，收发数据
		session->start();

		//这里必须要有这一步，如果没有这一步，那么session就会最后一个引用计数了，handle_accept函数调用完就会销毁了，
		m_sessions[session->uuid()] = session;
	}

	//处理完新的连接后，继续接收新的连接
	start_accept();
}

void Server::clear_session(const std::string& uuid)
{
	m_sessions.erase(uuid);
}
