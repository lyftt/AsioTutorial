#pragma once
#include "const.h"
#include <cstring>

class MsgNode
{
public:
	MsgNode(short max_len):m_total_len(max_len), m_cur_len(0), m_data(NULL)
	{
		m_data = new char[m_total_len + 1];   //����һ������\0
		m_data[m_total_len] = 0;
	}

	~MsgNode()
	{
		delete[] m_data;
	}

	/*��ո���*/
	void clear()
	{
		::memset(m_data, 0, m_total_len);
		m_cur_len = 0;
	}

	short m_total_len;
	short m_cur_len;
	char* m_data;
};

class RecvNode :public MsgNode
{
public:
	RecvNode(short max_len, short msg_id);

	short m_msg_id;
};

class SendNode:public MsgNode
{
public:
	SendNode(char* data, short max_len, short msg_id);

	short m_msg_id;
}
