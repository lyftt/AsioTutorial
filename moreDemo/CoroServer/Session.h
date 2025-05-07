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

	/*��ȡ�ײ�socket*/
	boost::asio::ip::tcp::socket& socket();

	/*��ȡuuid*/
	std::string& uuid();

	/*����*/
	void start();

	/*�ر�*/
	void close();

	/*����*/
	//���Ͳ����ʺ���Э��
	void send(const char* msg, short msg_len, short msg_id);

	/*����*/
	void send(const std::string& msg, short msg_id);

	/*������ɻص�����*/
	void handle_write(boost::system::error_code ec, std::size_t bytes_transferred, std::shared_ptr<Session> session);

private:
	boost::asio::io_context& m_ioc;
	Server* m_server;    //Session����Server
	boost::asio::ip::tcp::socket m_socket;
	std::string m_uuid;  //Session��ΨһID
	bool m_close;

	std::mutex m_send_lock;  //���Ͷ�����
	std::queue<std::shared_ptr<SendNode>> m_send_queue;   //���Ͷ���
	std::shared_ptr<RecvNode>  m_recv_msg_node;   //������Ϣ��ڵ�
	std::shared_ptr<MsgNode>   m_recv_head_node;  //������Ϣͷ���ڵ�
};

