#include "Session.h"
#include "Server.h"
#include <memory>
#include <functional>
#include <mutex>

void Session::start()
{
	memset(m_data, 0, max_length);

	//这里必须使用shared_from_this()来实现引用计数同步，而Session必须继承enable_shared_from_this
	//这是老旧的粘包处理方式
	//m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));

	//简单的粘包处理方式，读指定大小的头部，然后触发handle_read_head
	m_recv_head_node->clear();
	boost::asio::async_read(m_socket, boost::asio::buffer(m_recv_head_node->m_data, HEAD_TOTAL_LENGTH), std::bind(&Session::handle_read_head, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void Session::close()
{
	m_socket.close();   //主动关闭
	m_closed = true;
}

std::string& Session::uuid()
{
	return m_uuid;
}

void Session::send(char* msg, int max_length, short msg_id)
{
	//加锁保护
	std::lock_guard<std::mutex> guard(m_send_queue_lock);
	
	//获取现在发送队列的大小
	int queue_size = m_send_queue.size();
	if (queue_size > MAX_SEND_QUEUE)
	{
		std::cout << "send queue full, discard a msg" << std::endl;
		return;
	}

	//将消息放入发送节点
	m_send_queue.push(std::make_shared<SendNode>(msg, max_length, msg_id));

	if (queue_size > 0)
	{
		//前面的数据还没发送完则直接返回，因为写完成回调函数里会判断发送队列是否还有新的数据要发，如果有就会继续发
		//同时也会继续调用异步发送函数并注册写完成回调函数
		return;   
	}

	//如果此时前面的数据已经都发完了，则需要调用异步发送函数并注册回调函数
	auto& msg_node = m_send_queue.front();  //这里的处理比较特殊，必须从发送队列中取，因为发送队列中的MsgNode才加上了头部长度
	boost::asio::async_write(m_socket, boost::asio::buffer(msg_node->m_data, msg_node->m_total_len), std::bind(&Session::handle_write, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void Session::handle_read_head(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session)
{
	if (!ec)
	{
		//处理下异常情况，基本不可能
		if (bytes_transferred < HEAD_TOTAL_LENGTH)
		{
			std::cout << "read head length error" << std::endl;

			//关闭socket的事件处理，移除掉
			close();

			//清理服务器资源
			m_server->clear_session(m_uuid);
			return;
		}

		//正常的情况下
		std::cout << "read head length successful" << std::endl;

		//解析头部长度T+L
		short data_id = 0;
		short data_len = 0;
		memcpy(&data_id, m_recv_head_node->m_data, HEAD_ID_LENGTH);
		memcpy(&data_len, m_recv_head_node->m_data + HEAD_ID_LENGTH, HEAD_DATA_LENGTH);

		//字节序处理
		data_id = boost::asio::detail::socket_ops::network_to_host_short(data_id);
		data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);
		std::cout << "data id is " << data_id << std::endl;
		std::cout << "data len is " << data_len << std::endl;

		//检查头部长度是否合法
		if (data_len > max_length)
		{
			std::cout << "data len invalid" << std::endl;

			close();

			m_server->clear_session(m_uuid);
			return;
		}

		//走到这里说明一切正常
		//开始准备接收消息体
		m_recv_msg_node = std::make_shared<RecvNode>(data_len, data_id);
		//指定读多少数据才触发回调函数
		boost::asio::async_read(m_socket, boost::asio::buffer(m_recv_msg_node->m_data, data_len), std::bind(&Session::handle_read_msg, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
	}
	else
	{
		std::cout << "headle read head failed(simple), error is " << ec.what() << std::endl;

		//关闭连接
		close();

		m_server->clear_session(m_uuid);
	}
}

void Session::handle_read_msg(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session)
{
	if (!ec)
	{
		std::cout << "read msg body len:" << bytes_transferred << std::endl;

		//最好再判断下读到的字节数量是否是要求的数量

		//睡眠下模拟处理的情况，模拟服务器处理慢
		std::chrono::milliseconds dura(2000);
		std::this_thread::sleep_for(dura);

		//消息打印并回送给对端
		std::cout << "data body:" << m_recv_msg_node->m_data << std::endl;
		short data_id = 0;
		memcpy(&data_id, m_recv_head_node->m_data, HEAD_ID_LENGTH);
		data_id = boost::asio::detail::socket_ops::network_to_host_short(data_id);
		send(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len, data_id);

		//再次准备接收头部
		m_recv_head_node->clear();
		boost::asio::async_read(m_socket, boost::asio::buffer(m_recv_head_node->m_data, HEAD_TOTAL_LENGTH), std::bind(&Session::handle_read_head, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
	}
	else
	{
		std::cout << "headle read msg failed(simple), error is " << ec.what() << std::endl;

		//关闭连接
		close();

		m_server->clear_session(m_uuid);
	}
}

void Session::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session)
{
	if (!ec)
	{
		//已经移动的字节数，因为要在一大段字节流中进行处理
		int copy_len = 0;

		//还有未处理的数据
		while (bytes_transferred > 0)
		{
			//判读头部是否解析出来了，其实可以使用状态机来处理
			if (!m_head_parse_flag)
			{
				//剩余未处理的数据不够头部长度
				if (bytes_transferred + m_recv_head_node->m_cur_len < HEAD_TOTAL_LENGTH)
				{
					m_recv_head_node->append(m_data + copy_len, bytes_transferred);
					memset(m_data, 0, max_length);

					//继续监听后面的数据
					m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self_session));
					return;
				}

				//剩余未处理的数据比头部长度大
				int head_remain = HEAD_TOTAL_LENGTH - m_recv_head_node->m_cur_len;   //需要读取的头部剩余部分
				m_recv_head_node->append(m_data + copy_len, head_remain);
				copy_len += head_remain;
				bytes_transferred -= head_remain;   

				//此时头部数据已经获取成功了，但是整个字节流可能有很多报文，不只一个头部节点，我们收到一个完整的就先处理
				short data_id = 0;   //数据id
				short data_len = 0;  //数据长度
				memcpy(&data_id, m_recv_head_node->m_data, HEAD_ID_LENGTH);
				memcpy(&data_len, m_recv_head_node->m_data + HEAD_ID_LENGTH, HEAD_DATA_LENGTH);

				//网络字节序转换成网路字节序
				data_id = boost::asio::detail::socket_ops::network_to_host_short(data_id);
				data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);
				std::cout << "data id is " << data_id << std::endl;
				std::cout << "data len is " << data_len << std::endl;

				//判断头部长度是否合法
				if (data_len > max_length)
				{
					std::cout << "invalid data len is " << data_len << std::endl;
					m_server->clear_session(m_uuid);   //直接关闭连接
					return;
				}

				//开始处理消息体，先创建一个消息体的MsgNode
				m_recv_msg_node = std::make_shared<RecvNode>(data_len, data_id);

				//判断消息体是否完整，如果不完整，那么需要继续监听数据，并将目前收到的数据缓存到消息体节点中
				if (bytes_transferred < data_len)
				{
					m_recv_msg_node->append(m_data + copy_len, bytes_transferred);

					//继续监听数据
					memset(m_data, 0, max_length);
					m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self_session));
					
					//标记下头部已经处理完毕了
					m_head_parse_flag = true;

					return;
				}

				//到这里说明消息头部处理完了，剩余的未处理数据中也有完整的消息体
				m_recv_msg_node->append(m_data + copy_len, data_len);
				copy_len += data_len;
				bytes_transferred -= data_len;

				std::cout << "receive data:" << m_recv_msg_node->m_data << std::endl;

				//将收到的数据回送
				send(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len, data_id);

				//处理完一个报文，需要开始下一个，要将消息头部完整标记去除，重新开始
				m_head_parse_flag = false;

				//消息头部节点清空复用
				m_recv_head_node->clear();

				//如果已经将接收的数据都处理完了，那么需要继续监听
				if (bytes_transferred <= 0)
				{
					//继续监听数据
					memset(m_data, 0, max_length);
					m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self_session));

					return;
				}

				//到这里说明已经处理完了一个完整的消息，且还有剩余数据，那么需要继续循环处理
				continue;
			}

			//到这里说明消息头部处理完了，但是消息体还未处理完
			int remain_msg = m_recv_msg_node->m_total_len - m_recv_msg_node->m_cur_len;  //消息体还缺多少数据
			
			//剩余未处理的数据不够一个完整的消息体的，需要继续监听
			if (bytes_transferred < remain_msg)
			{
				//将这次收到的数据缓存下来
				m_recv_msg_node->append(m_data + copy_len, bytes_transferred);

				//继续监听后面的数据
				memset(m_data, 0, max_length);
				m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self_session));
				
				return;
			}

			//到这里说明剩下的未处理的数据满足了一个消息体的大小
			m_recv_msg_node->append(m_data + copy_len, remain_msg);
			copy_len += remain_msg;
			bytes_transferred -= remain_msg;

			std::cout << "receive data:" << m_recv_msg_node->m_data << std::endl;

			//回送消息
			short data_id = 0;
			memcpy(&data_id, m_recv_head_node->m_data, HEAD_ID_LENGTH);
			data_id = boost::asio::detail::socket_ops::network_to_host_short(data_id);
			send(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len, data_id);

			//处理完了一个完整的消息需要重新开始处理消息头
			m_head_parse_flag = false;

			//m_recv_msg_node不需要手动清理，因为每次处理到完整的消息头的时候都会重新创建一个m_recv_msg_node
			m_recv_head_node->clear();

			//如果已经没有剩余未处理的数据了，那么需要重新开始监听数据
			if (bytes_transferred <= 0)
			{
				memset(m_data, 0, max_length);
				m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self_session));

				return;
			}

			//走到这里说明处理完了一个完整的消息，但是还有剩余未处理的数据，那么需要继续循环处理
			continue;
		}
	}
	else
	{
		std::cout << "headle read failed, error is " << ec.what() << std::endl;
		
		//关闭连接
		close();

		m_server->clear_session(m_uuid);
	}
}

void Session::handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session)
{
	if (!ec)
	{
		//加锁保护
		std::lock_guard<std::mutex> guard(m_send_queue_lock);

		//写回调函数触发则说明一定全部发了，则将发送队列中的第一个MsgNode移除
		m_send_queue.pop();

		if (!m_send_queue.empty())
		{
			//发送队列中还有其他的消息需要发送，则继续发送
			auto& node = m_send_queue.front();

			boost::asio::async_write(m_socket, boost::asio::buffer(node->m_data, node->m_total_len), std::bind(&Session::handle_write, this, std::placeholders::_1, std::placeholders::_2, self_session));
		}
	}
	else
	{
		std::cout << "handle write failed, error is " << ec.what() << std::endl;

		//先主动关闭连接
		close();

		//改用了智能指针，不能再手动释放了，改为将智能指针从Server的数据结构中移除
		m_server->clear_session(m_uuid);
	}
}