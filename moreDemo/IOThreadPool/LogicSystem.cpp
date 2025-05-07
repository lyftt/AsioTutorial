#include "LogicSystem.h"

LogicSystem::~LogicSystem()
{
	m_stop_flag = true;
	m_consume.notify_one();
	m_worker_thread.join();
}

LogicSystem::LogicSystem():m_stop_flag(false)
{
	register_callback();
	m_worker_thread = std::thread(&LogicSystem::deal_msg, this);   //����ҵ���߼��߳�
}

void LogicSystem::post_msg_to_queue(std::shared_ptr<LogicNode> msg)
{
	//����
	std::unique_lock<std::mutex> unique_lk(m_mutex);
	if (m_stop_flag)
		return;

	m_msg_queue.push(msg);

	if (m_msg_queue.size() == 1)
	{
		m_consume.notify_one();
	}
}

void LogicSystem::register_callback()
{
	m_callbacks[MSG_HELLO] = std::bind(&LogicSystem::hello, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::hello(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data)
{
	std::cout << "logic system receive:" << msg_data << std::endl;

	session->send(msg_data, MSG_HELLO);   //���ͽ����client������ʹ��weak_ptr�Ƿ���ã���Ϊ������Ҫ���͵�ʱ�����ӿ����Ѿ��Ͽ���
}

void LogicSystem::deal_msg()
{
	for (;;)
	{
		//����
		std::unique_lock<std::mutex> unique_lk(m_mutex);

		while (m_msg_queue.empty() && !m_stop_flag)
		{
			m_consume.wait(unique_lk);
		}

		//�յ�ֹͣ����ˣ���ô��Ӧ��ȡ�����е����ݴ�������˳�
		if (m_stop_flag)
		{
			while (!m_msg_queue.empty())
			{
				auto msg = m_msg_queue.front();
				std::cout << "receive stop, now process msg_id:" << msg->m_recv_node->msg_id() << ", msg content:" << msg->m_recv_node->m_data << std::endl;

				auto call = m_callbacks.find(msg->m_recv_node->msg_id());
				if (call == m_callbacks.end())
				{
					m_msg_queue.pop();   //û�ҵ���Ϣ��Ӧ�Ĵ�����
					continue;
				}

				//���ҵ���Ϣ�Ĵ�����������ûص�����
				call->second(msg->m_session, msg->m_recv_node->msg_id(), std::string(msg->m_recv_node->m_data, msg->m_recv_node->m_cur_len));
				m_msg_queue.pop();
			}

			//�˳�
			break;
		}

		//û��ֹͣ�Ҷ�����������
		auto msg = m_msg_queue.front();
		std::cout << "receive stop, now process msg_id:" << msg->m_recv_node->msg_id() << ", msg content:" << msg->m_recv_node->m_data << std::endl;

		auto call = m_callbacks.find(msg->m_recv_node->msg_id());
		if (call == m_callbacks.end())
		{
			m_msg_queue.pop();   //û�ҵ���Ϣ��Ӧ�Ĵ�����
			continue;
		}
		
		call->second(msg->m_session, msg->m_recv_node->msg_id(), std::string(msg->m_recv_node->m_data, msg->m_recv_node->m_cur_len));
		m_msg_queue.pop();   //�������ķ�Χ̫��
	}
}
