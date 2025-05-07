#include "MsgNode.h"

RecvNode::RecvNode(short max_len, short msg_id):MsgNode(max_len),m_msg_id(msg_id)
{

}

SendNode::SendNode(const char* msg, short max_len, short msg_id):MsgNode(max_len + HEAD_TOTAL_LENGTH), m_msg_id(msg_id)
{
	//TLV中的T
	short msg_id_host = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
	memcpy(m_data, &msg_id_host, HEAD_ID_LENGTH);

	//TLV中的L
	short max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
	memcpy(m_data + HEAD_ID_LENGTH, &max_len_host, HEAD_DATA_LENGTH);

	//TLV中的V
	memcpy(m_data + HEAD_ID_LENGTH + HEAD_DATA_LENGTH, msg, max_len);
}
