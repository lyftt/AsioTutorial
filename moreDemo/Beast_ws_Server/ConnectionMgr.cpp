#include "ConnectionMgr.h"
#include "Connection.h"

ConnectionMgr& ConnectionMgr::get_instance()
{
	static ConnectionMgr instance;   //���ַ�ʽC++11�Ժ�ᱣ֤�̰߳�ȫ��
	return instance;
}

ConnectionMgr::~ConnectionMgr()
{
}

void ConnectionMgr::add_connection(std::shared_ptr<Connection> conn)
{
	m_conns_map[conn->uuid()] = conn;
}

void ConnectionMgr::remove_connection(const std::string id)
{
	m_conns_map.erase(id);
}

ConnectionMgr::ConnectionMgr()
{
}


