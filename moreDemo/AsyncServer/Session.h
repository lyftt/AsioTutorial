#pragma once
#include <iostream>
#include <boost/asio.hpp>

class Session
{
public:
	Session(boost::asio::io_context& ioc):m_socket(ioc)
	{

	}

	/*��ȡ�ڲ���socket������*/
	boost::asio::ip::tcp::socket& socket()
	{
		return m_socket;
	}

	/*��ʼ�����Կͻ��˵Ķ�д*/
	void start();

private:
	void handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred);
	void handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred);

private:
	boost::asio::ip::tcp::socket  m_socket;
	enum {max_length = 1024};
	char m_data[max_length];
};

class Server
{
public:
	Server(boost::asio::io_context& ioc, short port);

private:
	/*��ʼ����*/
	void start_accept();

	/*async_accept�Ļص�����*/
	void handle_accept(Session* session, const boost::system::error_code& ec);

private:
	boost::asio::io_context&  m_ioc;
	boost::asio::ip::tcp::acceptor m_ac;
};

