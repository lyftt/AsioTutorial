#pragma once
#include <iostream>
#include <boost/asio.hpp>

class Session
{
public:
	Session(boost::asio::io_context& ioc):m_socket(ioc)
	{

	}

	/*获取内部的socket的引用*/
	boost::asio::ip::tcp::socket& socket()
	{
		return m_socket;
	}

	/*开始监听对客户端的读写*/
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
	/*开始监听*/
	void start_accept();

	/*async_accept的回调函数*/
	void handle_accept(Session* session, const boost::system::error_code& ec);

private:
	boost::asio::io_context&  m_ioc;
	boost::asio::ip::tcp::acceptor m_ac;
};

