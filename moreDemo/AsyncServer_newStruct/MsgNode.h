#pragma once
#include <cstring>
#include <boost/asio.hpp>

#include "util.h"

/*������Ϣ�ͽ�����Ϣ��ʹ������ڵ�*/
class MsgNode
{
public:
    /*��󳤶Ȼ������Ϣͷ�����ȣ�ͷ����������̶�2�ֽ�*/
    /*�ù��캯������������Ϣ*/
    MsgNode(short max_len):m_cur_len(0), m_total_len(max_len)
    {
        m_data = new char[m_total_len + 1]{0};   //+1�����洢\0
        m_data[m_total_len] = '\0';
    }

    /*׷������*/
    short append(char* data, short len)
    {
        short remain_len = m_total_len - m_cur_len;
        short real_len = remain_len < len ? remain_len : len;
        memcpy(m_data + m_cur_len, data, real_len);
        m_cur_len += real_len;

        if (m_cur_len == m_total_len)
        {
            m_data[m_total_len] = '\0';
        }

        return real_len;
    }

    /*������ݣ������ظ�����*/
    void clear()
    {
        ::memset(m_data, 0, m_total_len);
        m_cur_len = 0;
    }

    ~MsgNode() {
        delete[] m_data;
    }

    short m_cur_len;     //���͵�����
    short m_total_len;   //�ܳ��ȣ�������Ϣͷ��
    char* m_data;
};

/*���սڵ�*/
class RecvNode : public MsgNode
{
public:
    RecvNode(short max_len, short msg_id);
private:
    short m_msg_id;
};

/*���ͽڵ�*/
class SendNode : public MsgNode
{
public:
    SendNode(const char* msg, short max_len, short msg_id);
private:
    short m_msg_id;
};

