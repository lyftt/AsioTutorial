#pragma once
#include "Session.h"
#include "MsgNode.h"
#include <memory>

class LogicNode
{
	friend class LogicSystem;

public:
	LogicNode(std::shared_ptr<Session> session, std::shared_ptr<RecvNode> recv_node) :m_session(session), m_recv_node(recv_node)
	{

	}

private:
	std::shared_ptr<Session>  m_session;
	std::shared_ptr<RecvNode> m_recv_node;
};
