#pragma once
#include "ConnectionMgr.h"
#include "boost/asio.hpp"

class WebSocketServer
{
public:
	WebSocketServer(boost::asio::io_context& ioc, unsigned short port);
	~WebSocketServer();

	void start_accept();

	/*²»ÔÊÐí¿½±´*/
	WebSocketServer(const WebSocketServer&) = delete;
	WebSocketServer& operator=(const WebSocketServer&) = delete;

private:
	boost::asio::io_context& m_ioc;
	boost::asio::ip::tcp::acceptor m_acp;
};

