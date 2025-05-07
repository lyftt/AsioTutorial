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

	//WebSocket����ײ�������asioʵ�ֵģ����Լ��������õĻ���asio����
	m_acp.async_accept(conn->socket(), [this, conn](boost::system::error_code ec) {
		try
		{
			if (!ec)  //û�д���˵���ײ��tcp����˳������
			{
				//��ʼ����websocket�����ӽ���������˵���֣�
				conn->async_accept();
			}
			else
			{
				std::cout << "async accept failed:" << ec.what() << std::endl;
			}

			//��������tcp����
			start_accept();
		}
		catch (const std::exception& e)
		{
			std::cout << "async accept exception:" << e.what() << std::endl;
		}
	});
}
