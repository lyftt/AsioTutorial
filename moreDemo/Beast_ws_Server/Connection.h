#pragma once
#include <iostream>
#include <ranges>
#include "boost/beast.hpp"
#include "boost/asio.hpp"
#include <memory>
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include <queue>
#include <mutex>
#include <string>

namespace net = boost::asio;
namespace beast = boost::beast;

/*
* websocket����
* 
*/
class Connection:public std::enable_shared_from_this<Connection>
{
public:

	/*���캯��*/
	Connection(net::io_context& ioc);

	/*uuid*/
	std::string uuid();

	/*����websocket�ײ��tcp socket*/
	net::ip::tcp::socket& socket();

	/*�ײ��tcp socket���ӽ���֮�󣬻���Ҫ��������websocket����*/
	void async_accept();

	/*��ʼ�շ��Է�����*/
	void start();

	/*����*/
	void async_send(const std::string& msg);

private:
	std::unique_ptr<beast::websocket::stream<beast::tcp_stream>> m_ws_ptr;  //websocket
	std::string m_uuid;                   //Ψһid
	net::io_context& m_ioc;               //io_context
	beast::flat_buffer m_recv_buffer;     //���ջ���
	std::queue<std::string> m_send_queue; //���Ͷ���
	std::mutex m_send_lock;               //���Ͷ�����
};

