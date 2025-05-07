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

//����ص���������
using FuncCallBack = std::function<void(std::shared_ptr<Session>, const short&, const std::string&)>;

//ҵ���߼���
class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;   //LogicSystem�Ĺ��캯����˽�еģ���ôSingleton��ͱ�������Ԫ����

public:
	/*����Ҫ�ܹ���ֻ��ָ����ʵ���������*/
	~LogicSystem();

	/*�ύҵ���߼���Ϣ*/
	void post_msg_to_queue(std::shared_ptr<LogicNode> msg);  

private:
	LogicSystem();      //˽�еĹ��캯��

	/*ע��ص�����*/
	void register_callback();

	/*���ԵĻص�����*/
	void hello(std::shared_ptr<Session> session, const short& msg_id, const std::string& msg_data);

	void deal_msg();

private:
	/*��Ϣ����*/
	std::queue<std::shared_ptr<LogicNode>> m_msg_queue;

	/*�������е���*/
	std::mutex m_mutex;

	/*����������������/�����߳�*/
	std::condition_variable m_consume;

	/*�����߳�*/
	std::thread m_worker_thread;

	/*ֹͣ��־*/
	bool m_stop_flag;

	/*��ͬ��Ϣ�Ļص�����*/
	std::map<short, FuncCallBack> m_callbacks;
};

