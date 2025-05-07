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
		//����һ��uuid��Ψһ��ʶһ��session��boost::uuids::random_generator()���ص���һ����������
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()();

		//ת���ַ���
		m_uuid = boost::uuids::to_string(a_uuid);

		//��Ϣͷ�����սڵ�Ĵ���
		m_recv_head_node = std::make_shared<MsgNode>(HEAD_LENGTH);
	}

	~Session()
	{
		std::cout << "Session:" << m_uuid << " cleand" << std::endl;
	}

	/*��ȡ�ڲ���socket������*/
	boost::asio::ip::tcp::socket& socket()
	{
		return m_socket;
	}

	/*��ʼ�����Կͻ��˵Ķ�д*/
	void start();

	/*�����ر�socket*/
	void close();

	/*��ȡ�ڲ���uuid*/
	std::string& uuid();

	/*���ͽӿ�*/
	void send(char* msg, int max_length);

private:
	/*����Session������ָ�����ӳ���������*/
	void handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session);
	void handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session);

private:
	boost::asio::ip::tcp::socket  m_socket;
	enum { max_length = 1024 * 2};
	char m_data[max_length];
	Server* m_server;   //������Server
	std::string m_uuid; //uuid��ÿ��Session����Ψһ��uuid

	/*�������*/
	std::queue<std::shared_ptr<MsgNode>> m_send_queue;  //���Ͷ��У�ÿ��Ԫ�ض���MsgNode
	std::mutex  m_send_queue_lock;                      //�������Ͷ��е�������Ϊһ�㶼�����߼�ҵ���ֳ��е��÷��ͽӿڣ���asio��io�߳�Ҳ����ʷ��Ͷ��У����Դ������ݾ���

	/*�������*/
	std::shared_ptr<MsgNode> m_recv_msg_node;    //���յ���Ϣ��
	std::shared_ptr<MsgNode> m_recv_head_node;   //���յ���Ϣͷ��
	bool m_head_parse_flag;                      //��Ϣͷ�������ɹ���־

	/*���ӹر����*/
	bool m_closed;
};
