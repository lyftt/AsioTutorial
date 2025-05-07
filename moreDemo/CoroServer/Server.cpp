#include "Server.h"
#include "IOServicePool.h"
#include "Server.h"
#include "Session.h"
#include "boost/asio.hpp"
#include <iostream>
#include <mutex>

Server::Server(boost::asio::io_context& ioc, short port) :m_ioc(ioc), m_port(port), m_acp(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	start_accept();
}

Server::~Server()
{

}

void Server::clear_session(const std::string& session)
{
	//ͬ����Ҫ��������
	std::lock_guard<std::mutex> lock(m_mutex);
	m_session.erase(session);
}

void Server::handle_accept(std::shared_ptr<Session> new_session, const boost::system::error_code& ec)
{
	if (!ec)
	{
		//��ʼsession�������շ��ʹ���
		new_session->start();

		//�����������ΪIOServicePool�Ƕ��̵߳ģ����е����Ӷ���¼�����m_session�У�
		//�Ǽ����߳̿��ܻ��⵽���������Ⲣ�����Ӵ�m_session��ɾ���������Ҫ���м�������
		std::lock_guard<std::mutex> lock(m_mutex);
		m_session.insert(std::make_pair(new_session->uuid(), new_session));

		//��������
		start_accept();
	}
	else
	{
		std::cout << "error:" << ec.what() << std::endl;
	}
}

void Server::start_accept()
{
	auto& ioc = IOServicePool::get_instance().get_io_service();
	auto new_session = std::make_shared<Session>(ioc, this);   //ʹ��pool�е�io_context

	m_acp.async_accept(new_session->socket(), std::bind(&Server::handle_accept, this, new_session, std::placeholders::_1));
}
