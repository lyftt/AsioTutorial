#include "LogicSystem.h"
#include "MsgNode.h"
#include "json/json.h"
#include "json/reader.h"
#include "Session.h"
#include <iostream>

LogicNode::LogicNode(std::shared_ptr<Session> session, std::shared_ptr<RecvNode> recv_node):m_session(session), m_recv_node(recv_node)
{

}

LogicNode::~LogicNode()
{

}

LogicSystem::~LogicSystem()
{
	m_stop = true;
	m_con.notify_all();
	m_thread.join();
}

LogicSystem::LogicSystem():m_stop(false)
{
	register_callback();

	m_thread = std::thread(&LogicSystem::deal_msg, this);   //启动线程
}

void LogicSystem::post_msg_to_queue(std::shared_ptr<LogicNode> msg)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_msg_queue.push(msg);

	if (m_msg_queue.size() == 1)
	{
		m_con.notify_one();
	}
}

LogicSystem& LogicSystem::get_instance()
{
	static LogicSystem instance;
	return instance;
}

void LogicSystem::deal_msg()
{
	for (; ;)
	{
		std::unique_lock<std::mutex> lock(m_mutex);   //这里的锁范围较大
		while (m_msg_queue.empty() && !m_stop)
		{
			m_con.wait(lock);
		}

		//停止了
		if (m_stop)
		{
			//处理完剩下的
			while (!m_msg_queue.empty())
			{
				auto node = m_msg_queue.front();
				auto call_iter = m_callbacks.find(node->m_recv_node->m_msg_id);
				if (call_iter == m_callbacks.end())
				{
					m_msg_queue.pop();
					continue;
				}

				call_iter->second(node->m_session, node->m_recv_node->m_msg_id, std::string(node->m_recv_node->m_data, node->m_recv_node->m_total_len));

				m_msg_queue.pop();
			}

			break;
		}

		//队列非空且没有停止，则取出消息处理
		auto node = m_msg_queue.front();
		m_msg_queue.pop();
		lock.unlock();   //减小锁的范围

		auto call_iter = m_callbacks.find(node->m_recv_node->m_msg_id);
		if (call_iter == m_callbacks.end())
		{
			continue;
		}

		call_iter->second(node->m_session, node->m_recv_node->m_msg_id, std::string(node->m_recv_node->m_data, node->m_recv_node->m_total_len));
	}
}

void LogicSystem::register_callback()
{
	m_callbacks[MSG_HELLO_WORD] = std::bind(&LogicSystem::HelloCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::HelloCallBack(std::shared_ptr<Session> session, const short msg_id, const std::string& msg)
{
	Json::Reader reader;
	Json::Value root;
	
	//解析
	reader.parse(msg, root);
	std::cout << "msg_id:" << msg_id << ", data:" << msg.c_str() << std::endl;
	std::cout << "id:" << root["id"].asInt() << std::endl;
	std::cout << "data:" << root["data"].asString() << std::endl;

	root["data"] = "received, thanks";

	std::string return_str = root.toStyledString();
	session->send(return_str, root["id"].asInt());
}
