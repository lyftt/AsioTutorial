#include "MsgNode.h"
#include "boost/asio.hpp"

RecvNode::RecvNode(short max_len, short msg_id):MsgNode(max_len), m_msg_id(msg_id)
{
}

SendNode::SendNode(const char* data, short max_len, short msg_id):MsgNode(max_len + HEAD_TOTAL_LEN), m_msg_id(msg_id)
{
	//网络字节序转换
	short msg_id_net = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
	short max_len_net = boost::asio::detail::socket_ops::host_to_network_short(max_len);

	//建造发送节点
	::memcpy(m_data, &msg_id_net, sizeof(msg_id_net));
	::memcpy(m_data + sizeof(msg_id_net), &max_len_net, sizeof(max_len_net));
	::memcpy(m_data + HEAD_TOTAL_LEN, data, max_len);
}
