#pragma once
#include <cstring>

class MsgNode
{
    friend class Session;   //����Session����ֱ��ʹ��
public:
    MsgNode(char* msg, int max_len):m_cur_len(0), m_max_len(max_len)
    {
        m_data = new char[max_len];
        memcpy(m_data, msg, max_len);
    }

    ~MsgNode() {
        delete[] m_data;
    }

private:
    int m_cur_len;
    int m_max_len;
    char* m_data;
};
