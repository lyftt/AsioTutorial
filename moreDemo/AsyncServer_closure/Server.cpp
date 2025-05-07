#include "Server.h"
#include "Session.h"

Server::Server(boost::asio::io_context& ioc, short port) :m_ioc(ioc), m_ac(m_ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	start_accept();
}

void Server::start_accept()
{
	//����ԭʼָ�룬����Ҫ�ĳ�ʹ������ָ��
	//Session* session = new Session(m_ioc);
	//��������ָ��
	std::shared_ptr<Session> session = std::make_shared<Session>(m_ioc, this);

	//�ص�����ֻ����һ��������asio����ģ�����������Ҫ���µ�һ������
	//����std::bind������һ���ɵ��ö�����������ڲ��´��һ��session�������ָ�룬��������ü���+1
	m_ac.async_accept(session->socket(), std::bind(&Server::handle_accept, this, session, std::placeholders::_1));
}

void Server::handle_accept(std::shared_ptr<Session> session, const boost::system::error_code& ec)
{
	if (ec)
	{
		std::cout << "handle accept error" << std::endl;

		//�ͷŵ�Session���ĳ�����ָ��֮��Ͳ���ʹ��delete��
		//delete this;

		//��������⣬��ôʲô�����øɣ�����Ҫ����m_sessions��session������������handle_accept������ͻ����
	}
	else
	{
		//����session�Ự���շ�����
		session->start();

		//�������Ҫ����һ�������û����һ������ôsession�ͻ����һ�����ü����ˣ�handle_accept����������ͻ������ˣ�
		//�������ö��ص�ɶ�Ķ����������
		m_sessions[session->uuid()] = session;
	}

	//�������µ����Ӻ󣬼��������µ�����
	start_accept();
}

void Server::clear_session(const std::string& uuid)
{
	m_sessions.erase(uuid);
}
