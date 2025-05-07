#pragma once
#include <memory>
#include "boost/asio.hpp"
#include <string>
#include <queue>
#include <mutex>
#include "MsgNode.h"

class Server;

class Session: public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& ioc, Server* svr);
	~Session();

	/*获取底层socket*/
	boost::asio::ip::tcp::socket& socket();

	/*获取uuid*/
	std::string& uuid();

	/*启动*/
	void start();

	/*关闭*/
	void close();

	/*发送*/
	//发送并不适合用协程
	void send(const char* msg, short msg_len, short msg_id);

	/*发送*/
	void send(const std::string& msg, short msg_id);

	/*发送完成回调函数*/
	void handle_write(boost::system::error_code ec, std::size_t bytes_transferred, std::shared_ptr<Session> session);

private:
	boost::asio::io_context& m_ioc;
	Server* m_server;    //Session所属Server
	boost::asio::ip::tcp::socket m_socket;
	std::string m_uuid;  //Session的唯一ID
	bool m_close;

	std::mutex m_send_lock;  //发送队列锁
	std::queue<std::shared_ptr<SendNode>> m_send_queue;   //发送队列
	std::shared_ptr<RecvNode>  m_recv_msg_node;   //接收消息体节点
	std::shared_ptr<MsgNode>   m_recv_head_node;  //接收消息头部节点
};

