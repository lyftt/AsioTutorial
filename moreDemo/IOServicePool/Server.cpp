#include "Server.h"
#include "Session.h"
#include "IOServicePool.h"

Server::Server(boost::asio::io_context& ioc, short port) :m_ioc(ioc), m_ac(m_ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	start_accept();
}

void Server::start_accept()
{
	//����ʹ��IOServicePool�е�io_context
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
		//����session�Ự���շ�����
		session->start();

		//�������Ҫ����һ�������û����һ������ôsession�ͻ����һ�����ü����ˣ�handle_accept����������ͻ������ˣ�
		m_sessions[session->uuid()] = session;
	}

	//�������µ����Ӻ󣬼��������µ�����
	start_accept();
}

void Server::clear_session(const std::string& uuid)
{
	m_sessions.erase(uuid);
}
