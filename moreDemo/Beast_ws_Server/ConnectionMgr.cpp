#include "ConnectionMgr.h"
#include "Connection.h"

ConnectionMgr& ConnectionMgr::get_instance()
{
	static ConnectionMgr instance;   //这种方式C++11以后会保证线程安全性
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


