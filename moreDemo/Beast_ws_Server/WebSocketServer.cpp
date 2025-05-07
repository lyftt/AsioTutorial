#include "WebSocketServer.h"
#include "Connection.h"
#include <iostream>

WebSocketServer::WebSocketServer(boost::asio::io_context& ioc, unsigned short port):m_ioc(ioc), m_acp(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	std::cout << "server start on " << port << std::endl;
}

WebSocketServer::~WebSocketServer()
{
}

void WebSocketServer::start_accept()
{
	auto conn = std::make_shared<Connection>(m_ioc);

	//WebSocket服务底层是以来asio实现的，所以监听连接用的还是asio那套
	m_acp.async_accept(conn->socket(), [this, conn](boost::system::error_code ec) {
		try
		{
			if (!ec)  //没有错误说明底层的tcp连接顺利建立
			{
				//开始进行websocket的连接建立（或者说握手）
				conn->async_accept();
			}
			else
			{
				std::cout << "async accept failed:" << ec.what() << std::endl;
			}

			//继续监听tcp连接
			start_accept();
		}
		catch (const std::exception& e)
		{
			std::cout << "async accept exception:" << e.what() << std::endl;
		}
	});
}
