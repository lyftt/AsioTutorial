#pragma once
#include <memory>
#include <queue>
#include <map>
#include <functional>
#include <thread>
#include <mutex>

class RecvNode;
class Session;

typedef std::function<void(std::shared_ptr<Session>, const short, const std::string&)> FuncCallBack;

class LogicNode
{
public:
	LogicNode(std::shared_ptr<Session> session, std::shared_ptr<RecvNode> recv_node);
	~LogicNode();

	std::shared_ptr<Session> m_session;
	std::shared_ptr<RecvNode> m_recv_node;
};

class LogicSystem
{
public:
	~LogicSystem();
	LogicSystem(const LogicSystem&) = delete;
	LogicSystem& operator=(const LogicSystem&) = delete;

	void post_msg_to_queue(std::shared_ptr<LogicNode> msg);

	static LogicSystem& get_instance();

private:
	LogicSystem();

	void deal_msg();

	void register_callback();

	void HelloCallBack(std::shared_ptr<Session> session, const short msg_id, const std::string& msg);

private:
	std::thread m_thread;
	std::queue<std::shared_ptr<LogicNode>>  m_msg_queue;
	std::mutex m_mutex;
	std::condition_variable m_con;
	bool m_stop;
	std::map<short, FuncCallBack>  m_callbacks;
};

