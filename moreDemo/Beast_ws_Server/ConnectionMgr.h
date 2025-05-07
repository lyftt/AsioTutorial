#pragma once
#include <unordered_map>
#include <memory>
#include <string>

class Connection;  //ǰ������

/*
* ���ӹ�����
* 
*/
class ConnectionMgr
{
public:
	static ConnectionMgr& get_instance();
	~ConnectionMgr();

	void add_connection(std::shared_ptr<Connection> conn);
	void remove_connection(const std::string id);

	ConnectionMgr(const ConnectionMgr&) = delete;
	ConnectionMgr& operator=(const ConnectionMgr&) = delete;

private:
	ConnectionMgr();   //���캯��˽��

private:
	std::unordered_map<std::string, std::shared_ptr<Connection>> m_conns_map;
};

