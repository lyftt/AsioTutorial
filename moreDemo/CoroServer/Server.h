#pragma once
#include <map>
#include <mutex>
#include <memory>
#include <string>
#include "boost/asio.hpp"

class Session;

class Server
{
public:
	Server(boost::asio::io_context& ioc, short port);
	~Server();

	void clear_session(const std::string& session);

private:
	void handle_accept(std::shared_ptr<Session> new_session, const boost::system::error_code& ec);
	void start_accept();

private:
	boost::asio::io_context& m_ioc;    //上下文
	short m_port;                   //端口号
	boost::asio::ip::tcp::acceptor m_acp;
	std::map<std::string, std::shared_ptr<Session>> m_session;

	std::mutex m_mutex;
};

