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
	m_worker_thread = std::thread(&LogicSystem::deal_msg, this);   //启动业务逻辑线程
}

void LogicSystem::post_msg_to_queue(std::shared_ptr<LogicNode> msg)
{
	//上锁
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

	session->send(msg_data, MSG_HELLO);   //发送结果给client，这里使用weak_ptr是否更好？因为处理完要发送的时候连接可能已经断开了
}

void LogicSystem::deal_msg()
{
	for (;;)
	{
		//上锁
		std::unique_lock<std::mutex> unique_lk(m_mutex);

		while (m_msg_queue.empty() && !m_stop_flag)
		{
			m_consume.wait(unique_lk);
		}

		//收到停止标记了，那么就应该取出所有的数据处理完后退出
		if (m_stop_flag)
		{
			while (!m_msg_queue.empty())
			{
				auto msg = m_msg_queue.front();
				std::cout << "receive stop, now process msg_id:" << msg->m_recv_node->msg_id() << ", msg content:" << msg->m_recv_node->m_data << std::endl;

				auto call = m_callbacks.find(msg->m_recv_node->msg_id());
				if (call == m_callbacks.end())
				{
					m_msg_queue.pop();   //没找到消息对应的处理函数
					continue;
				}

				//能找到消息的处理函数，则调用回调函数
				call->second(msg->m_session, msg->m_recv_node->msg_id(), std::string(msg->m_recv_node->m_data, msg->m_recv_node->m_cur_len));
				m_msg_queue.pop();
			}

			//退出
			break;
		}

		//没有停止且队列中有数据
		auto msg = m_msg_queue.front();
		std::cout << "receive stop, now process msg_id:" << msg->m_recv_node->msg_id() << ", msg content:" << msg->m_recv_node->m_data << std::endl;

		auto call = m_callbacks.find(msg->m_recv_node->msg_id());
		if (call == m_callbacks.end())
		{
			m_msg_queue.pop();   //没找到消息对应的处理函数
			continue;
		}
		
		call->second(msg->m_session, msg->m_recv_node->msg_id(), std::string(msg->m_recv_node->m_data, msg->m_recv_node->m_cur_len));
		m_msg_queue.pop();   //这里锁的范围太大
	}
}
