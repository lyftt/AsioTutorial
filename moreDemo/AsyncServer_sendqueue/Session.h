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
	Session(boost::asio::io_context& ioc, Server* server) :m_socket(ioc), m_server(server)
	{
		//生成一个uuid来唯一标识一个session，boost::uuids::random_generator()返回的是一个函数对象
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()();

		//转成字符串
		m_uuid = boost::uuids::to_string(a_uuid);
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
	enum { max_length = 1024 };
	char m_data[max_length];
	Server* m_server;   //所属的Server
	std::string m_uuid; //uuid，每个Session都有唯一的uuid
	std::queue<std::shared_ptr<MsgNode>> m_send_queue;  //发送队列，每个元素都是MsgNode
	std::mutex  m_send_queue_lock;                      //保护发送队列的锁，因为一般都会在逻辑业务现场中调用发送接口，而asio的io线程也会访问发送队列，所以存在数据竞争
};
