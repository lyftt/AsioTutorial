#pragma once
#include "Singleton.h"
#include <thread>
#include <queue>
#include "Session.h"
#include <map>
#include <functional>
#include <string>
#include "util.h"
#include "LogicNode.h"

//定义回调函数类型
using FuncCallBack = std::function<void(std::shared_ptr<Session>, const short&, const std::string&)>;

//业务逻辑类
class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;   //LogicSystem的构造函数是私有的，那么Singleton类就必须是友元才行

public:
	/*必须要能够让只能指针访问到析构函数*/
	~LogicSystem();

	/*提交业务逻辑消息*/
	void post_msg_to_queue(std::shared_ptr<LogicNode> msg);  

private:
	LogicSystem();      //私有的构造函数

	/*注册回调函数*/
	void register_callback();

	/*测试的回调函数*/
	void hello(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);

	void deal_msg();

private:
	/*消息队列*/
	std::queue<std::shared_ptr<LogicNode>> m_msg_queue;

	/*保护队列的锁*/
	std::mutex m_mutex;

	/*条件变量用来挂起/唤醒线程*/
	std::condition_variable m_consume;

	/*工作线程*/
	std::thread m_worker_thread;

	/*停止标志*/
	bool m_stop_flag;

	/*不同消息的回调函数*/
	std::map<short, FuncCallBack> m_callbacks;
};

