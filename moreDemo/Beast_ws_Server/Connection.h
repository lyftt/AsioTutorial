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
* websocket连接
* 
*/
class Connection:public std::enable_shared_from_this<Connection>
{
public:

	/*构造函数*/
	Connection(net::io_context& ioc);

	/*uuid*/
	std::string uuid();

	/*返回websocket底层的tcp socket*/
	net::ip::tcp::socket& socket();

	/*底层的tcp socket连接建立之后，还需要升级建立websocket连接*/
	void async_accept();

	/*开始收发对方数据*/
	void start();

	/*发送*/
	void async_send(const std::string& msg);

private:
	std::unique_ptr<beast::websocket::stream<beast::tcp_stream>> m_ws_ptr;  //websocket
	std::string m_uuid;                   //唯一id
	net::io_context& m_ioc;               //io_context
	beast::flat_buffer m_recv_buffer;     //接收缓冲
	std::queue<std::string> m_send_queue; //发送队列
	std::mutex m_send_lock;               //发送队列锁
};

