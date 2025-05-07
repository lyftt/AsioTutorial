#pragma once
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "MsgNode.h"

class Server;

class Session : std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& ioc, Server* server) :m_socket(ioc), m_server(server), m_head_parse_flag(false), m_closed(false)
	{
		//生成一个uuid来唯一标识一个session，boost::uuids::random_generator()返回的是一个函数对象
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()();

		//转成字符串
		m_uuid = boost::uuids::to_string(a_uuid);

		//消息头部接收节点的创建
		m_recv_head_node = std::make_shared<MsgNode>(HEAD_LENGTH);
	}

	~Session()
	{
		std::cout << "Session:" << m_uuid << " cleand" << std::endl;
	}

	/*获取内部的socket的引用*/
	boost::asio::ip::tcp::socket& socket()
	{
		return m_socket;
	}

	/*开始监听对客户端的读写*/
	void start();

	/*主动关闭socket*/
	void close();

	/*获取内部的uuid*/
	std::string& uuid();

	/*发送接口*/
	void send(char* msg, int max_length);

private:
	/*增加Session的智能指针来延长生命周期*/
	void handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session);
	void handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session);

private:
	boost::asio::ip::tcp::socket  m_socket;
	enum { max_length = 1024 * 2};
	char m_data[max_length];
	Server* m_server;   //所属的Server
	std::string m_uuid; //uuid，每个Session都有唯一的uuid

	/*发送相关*/
	std::queue<std::shared_ptr<MsgNode>> m_send_queue;  //发送队列，每个元素都是MsgNode
	std::mutex  m_send_queue_lock;                      //保护发送队列的锁，因为一般都会在逻辑业务现场中调用发送接口，而asio的io线程也会访问发送队列，所以存在数据竞争

	/*接收相关*/
	std::shared_ptr<MsgNode> m_recv_msg_node;    //接收的消息体
	std::shared_ptr<MsgNode> m_recv_head_node;   //接收的消息头部
	bool m_head_parse_flag;                      //消息头部解析成功标志

	/*连接关闭相关*/
	bool m_closed;
};
