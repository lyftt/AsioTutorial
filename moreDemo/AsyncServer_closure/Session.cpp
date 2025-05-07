#include "Session.h"
#include "Server.h"
#include <memory>
#include <functional>
#include <mutex>

void Session::start()
{
	memset(m_data, 0, max_length);

	//�����첽���ص���һ���յ����ݾͻᴥ��Session::handle_read����
	//�������ʹ��shared_from_this()��ʵ�����ü���ͬ������Session����̳�enable_shared_from_this
	m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

std::string& Session::uuid()
{
	return m_uuid;
}

void Session::send(char* msg, int max_length)
{
	bool pending = false;

	//��������
	std::lock_guard<std::mutex> guard(m_send_queue_lock);

	if(m_send_queue.size() > 0)
	{
		pending = true;   //˵��ǰ������ݻ�û������
	}

	m_send_queue.push(std::make_shared<MsgNode>(msg, max_length));

	if (pending)
	{
		//ǰ������ݻ�û��������ֱ�ӷ��أ���Ϊд��ɻص���������жϷ��Ͷ����Ƿ����µ�����Ҫ��������оͻ������
		//ͬʱҲ����������첽���ͺ�����ע��д��ɻص�����
		return;   
	}

	//�����ʱǰ��������Ѿ��������ˣ�����Ҫ�����첽���ͺ�����ע��ص�����
	boost::asio::async_write(m_socket, boost::asio::buffer(msg, max_length), std::bind(&Session::handle_write, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void Session::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session)
{
	if (ec)
	{
		std::cout << "handle read error" << std::endl;

		//����������ָ�룬�������ֶ��ͷ��ˣ���Ϊ������ָ���Server�����ݽṹ���Ƴ�
		m_server->clear_session(m_uuid);

		return;
	}

	//û�з�������
	std::cout << "received:" << m_data << std::endl;

	//��������
	send(m_data, bytes_transferred);

	//�ٴε���async_read_some()��������
	memset(m_data, 0, max_length);
	//m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, std::placeholders::_1, std::placeholders::_2, self_session));
}

void Session::handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session)
{
	if (ec)
	{
		std::cout << "handle write error" << std::endl;

		//����������ָ�룬�������ֶ��ͷ��ˣ���Ϊ������ָ���Server�����ݽṹ���Ƴ�
		m_server->clear_session(m_uuid);

		return;
	}

	//��������
	std::lock_guard<std::mutex> guard(m_send_queue_lock);

	//д�ص�����������˵��һ��ȫ�����ˣ��򽫷��Ͷ����еĵ�һ��MsgNode�Ƴ�
	m_send_queue.pop();

	if (!m_send_queue.empty())
	{
		//���Ͷ����л�����������Ϣ��Ҫ���ͣ����������
		auto& node = m_send_queue.front();

		boost::asio::async_write(m_socket, boost::asio::buffer(node->m_data, node->m_max_len), std::bind(&Session::handle_write, this, std::placeholders::_1, std::placeholders::_2, self_session));
	}
}