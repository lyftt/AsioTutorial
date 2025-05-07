#pragma once
#include <iostream>
#include <map>
#include <memory>
#include <boost/asio.hpp>

class Session;

class Server
{
public:
	Server(boost::asio::io_context& ioc, short port);

	/*�Ƴ��ض���Session������ָ��*/
	void clear_session(const std::string& uuid);

private:
	/*��ʼ����*/
	void start_accept();

	/*async_accept�Ļص�����*/
	void handle_accept(std::shared_ptr<Session> session, const boost::system::error_code& ec);

private:
	boost::asio::io_context& m_ioc;
	boost::asio::ip::tcp::acceptor m_ac;

	std::map<std::string, std::shared_ptr<Session>> m_sessions;   //��������Session
};
