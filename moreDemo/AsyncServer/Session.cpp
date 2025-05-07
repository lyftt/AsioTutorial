#include "Session.h"
#include <memory>
#include <functional>

void Session::start()
{
	memset(m_data, 0, max_length);

	//�����첽���ص���һ���յ����ݾͻᴥ��Session::handle_read����
	m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2));
}

void Session::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if (ec)
	{
		std::cout << "handle read error" << std::endl;

		//�ͷŵ�Session�Լ�����Ϊ�Է��Ѿ������ˣ�����������
		delete this;

		return;
	}

	//û�з�������
	std::cout << "received:" << m_data << std::endl;

	//�����������ôд����ô���client�ڵ���async_write֮ǰ�˳�����ᵼ��handle_read��handle_write�������������ᷢ��ec�Ǵ��ڴ���ģ�����delete2��
	//��ʵ������ˣ������������ص������е�һ���ȱ������ˣ���this�ͷ��ˣ���ô��ʱSession�Ѿ��������ˣ��ڴ��Ѿ���Ч�ˣ�֮��ĵڶ��ε��ûص����������
	//�����κβ�����Ա�����Ĳ�����������������
	//����Session������������һ������
	//m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2));

	//���ͻ�������
	//m_socket.async_send(boost::asio::buffer(m_data, bytes_transferred), std::bind(Session::handle_write, this, std::placeholders::_1));
	boost::asio::async_write(m_socket, boost::asio::buffer(m_data, bytes_transferred), std::bind(&Session::handle_write, this, std::placeholders::_1, std::placeholders::_2));
}

void Session::handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if (ec)
	{
		std::cout << "handle write error" << std::endl;

		//�ͷŵ�Session�Լ�����Ϊ�Է��Ѿ������ˣ�����������
		delete this;

		return;
	}

	//û�з�������
	memset(m_data, 0, max_length);

	//��������������
	m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2));
}

Server::Server(boost::asio::io_context& ioc, short port):m_ioc(ioc),m_ac(m_ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	start_accept();
}

void Server::start_accept()
{
	//��ʼ����
	Session* session = new Session(m_ioc);

	//�ص�����ֻ����һ��������asio����ģ�����������Ҫ���µ�һ������
	m_ac.async_accept(session->socket(), std::bind(&Server::handle_accept, this, session, std::placeholders::_1));
}

void Server::handle_accept(Session* session, const boost::system::error_code& ec)
{
	if (ec)
	{
		std::cout << "handle accept error" << std::endl;

		//�ͷŵ�Session
		delete this;
	}
	else
	{
		//����session�Ự���շ�����
		session->start();
	}

	//�������µ����Ӻ󣬼��������µ�����
	start_accept();
}
