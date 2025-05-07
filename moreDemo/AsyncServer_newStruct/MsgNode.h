#pragma once
#include <cstring>
#include <boost/asio.hpp>

#include "util.h"

/*发送消息和接收消息都使用这个节点*/
class MsgNode
{
public:
    /*最大长度会加上消息头部长度，头部长度这里固定2字节*/
    /*该构造函数用来发送消息*/
    MsgNode(short max_len):m_cur_len(0), m_total_len(max_len)
    {
        m_data = new char[m_total_len + 1]{0};   //+1用来存储\0
        m_data[m_total_len] = '\0';
    }

    /*追加数据*/
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

    /*清除数据，方便重复利用*/
    void clear()
    {
        ::memset(m_data, 0, m_total_len);
        m_cur_len = 0;
    }

    ~MsgNode() {
        delete[] m_data;
    }

    short m_cur_len;     //发送到哪了
    short m_total_len;   //总长度，包括消息头部
    char* m_data;
};

/*接收节点*/
class RecvNode : public MsgNode
{
public:
    RecvNode(short max_len, short msg_id);
private:
    short m_msg_id;
};

/*发送节点*/
class SendNode : public MsgNode
{
public:
    SendNode(const char* msg, short max_len, short msg_id);
private:
    short m_msg_id;
};

