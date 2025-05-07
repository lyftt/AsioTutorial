#include "Session.h"
#include "Server.h"
#include <memory>
#include <functional>
#include <mutex>

void Session::start()
{
	memset(m_data, 0, max_length);

	//�������ʹ��shared_from_this()��ʵ�����ü���ͬ������Session����̳�enable_shared_from_this
	//�����Ͼɵ�ճ������ʽ
	//m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));

	//�򵥵�ճ������ʽ����ָ����С��ͷ����Ȼ�󴥷�handle_read_head
	m_recv_head_node->clear();
	boost::asio::async_read(m_socket, boost::asio::buffer(m_recv_head_node->m_data, HEAD_TOTAL_LENGTH), std::bind(&Session::handle_read_head, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void Session::close()
{
	m_socket.close();   //�����ر�
	m_closed = true;
}

std::string& Session::uuid()
{
	return m_uuid;
}

void Session::send(char* msg, int max_length, short msg_id)
{
	//��������
	std::lock_guard<std::mutex> guard(m_send_queue_lock);
	
	//��ȡ���ڷ��Ͷ��еĴ�С
	int queue_size = m_send_queue.size();
	if (queue_size > MAX_SEND_QUEUE)
	{
		std::cout << "send queue full, discard a msg" << std::endl;
		return;
	}

	//����Ϣ���뷢�ͽڵ�
	m_send_queue.push(std::make_shared<SendNode>(msg, max_length, msg_id));

	if (queue_size > 0)
	{
		//ǰ������ݻ�û��������ֱ�ӷ��أ���Ϊд��ɻص���������жϷ��Ͷ����Ƿ����µ�����Ҫ��������оͻ������
		//ͬʱҲ����������첽���ͺ�����ע��д��ɻص�����
		return;   
	}

	//�����ʱǰ��������Ѿ��������ˣ�����Ҫ�����첽���ͺ�����ע��ص�����
	auto& msg_node = m_send_queue.front();  //����Ĵ���Ƚ����⣬����ӷ��Ͷ�����ȡ����Ϊ���Ͷ����е�MsgNode�ż�����ͷ������
	boost::asio::async_write(m_socket, boost::asio::buffer(msg_node->m_data, msg_node->m_total_len), std::bind(&Session::handle_write, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void Session::handle_read_head(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session)
{
	if (!ec)
	{
		//�������쳣���������������
		if (bytes_transferred < HEAD_TOTAL_LENGTH)
		{
			std::cout << "read head length error" << std::endl;

			//�ر�socket���¼������Ƴ���
			close();

			//�����������Դ
			m_server->clear_session(m_uuid);
			return;
		}

		//�����������
		std::cout << "read head length successful" << std::endl;

		//����ͷ������T+L
		short data_id = 0;
		short data_len = 0;
		memcpy(&data_id, m_recv_head_node->m_data, HEAD_ID_LENGTH);
		memcpy(&data_len, m_recv_head_node->m_data + HEAD_ID_LENGTH, HEAD_DATA_LENGTH);

		//�ֽ�����
		data_id = boost::asio::detail::socket_ops::network_to_host_short(data_id);
		data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);
		std::cout << "data id is " << data_id << std::endl;
		std::cout << "data len is " << data_len << std::endl;

		//���ͷ�������Ƿ�Ϸ�
		if (data_len > max_length)
		{
			std::cout << "data len invalid" << std::endl;

			close();

			m_server->clear_session(m_uuid);
			return;
		}

		//�ߵ�����˵��һ������
		//��ʼ׼��������Ϣ��
		m_recv_msg_node = std::make_shared<RecvNode>(data_len, data_id);
		//ָ�����������ݲŴ����ص�����
		boost::asio::async_read(m_socket, boost::asio::buffer(m_recv_msg_node->m_data, data_len), std::bind(&Session::handle_read_msg, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
	}
	else
	{
		std::cout << "headle read head failed(simple), error is " << ec.what() << std::endl;

		//�ر�����
		close();

		m_server->clear_session(m_uuid);
	}
}

void Session::handle_read_msg(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session)
{
	if (!ec)
	{
		std::cout << "read msg body len:" << bytes_transferred << std::endl;

		//������ж��¶������ֽ������Ƿ���Ҫ�������

		//˯����ģ�⴦��������ģ�������������
		std::chrono::milliseconds dura(2000);
		std::this_thread::sleep_for(dura);

		//��Ϣ��ӡ�����͸��Զ�
		std::cout << "data body:" << m_recv_msg_node->m_data << std::endl;
		short data_id = 0;
		memcpy(&data_id, m_recv_head_node->m_data, HEAD_ID_LENGTH);
		data_id = boost::asio::detail::socket_ops::network_to_host_short(data_id);
		send(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len, data_id);

		//�ٴ�׼������ͷ��
		m_recv_head_node->clear();
		boost::asio::async_read(m_socket, boost::asio::buffer(m_recv_head_node->m_data, HEAD_TOTAL_LENGTH), std::bind(&Session::handle_read_head, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
	}
	else
	{
		std::cout << "headle read msg failed(simple), error is " << ec.what() << std::endl;

		//�ر�����
		close();

		m_server->clear_session(m_uuid);
	}
}

void Session::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session)
{
	if (!ec)
	{
		//�Ѿ��ƶ����ֽ�������ΪҪ��һ����ֽ����н��д���
		int copy_len = 0;

		//����δ���������
		while (bytes_transferred > 0)
		{
			//�ж�ͷ���Ƿ���������ˣ���ʵ����ʹ��״̬��������
			if (!m_head_parse_flag)
			{
				//ʣ��δ��������ݲ���ͷ������
				if (bytes_transferred + m_recv_head_node->m_cur_len < HEAD_TOTAL_LENGTH)
				{
					m_recv_head_node->append(m_data + copy_len, bytes_transferred);
					memset(m_data, 0, max_length);

					//�����������������
					m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self_session));
					return;
				}

				//ʣ��δ��������ݱ�ͷ�����ȴ�
				int head_remain = HEAD_TOTAL_LENGTH - m_recv_head_node->m_cur_len;   //��Ҫ��ȡ��ͷ��ʣ�ಿ��
				m_recv_head_node->append(m_data + copy_len, head_remain);
				copy_len += head_remain;
				bytes_transferred -= head_remain;   

				//��ʱͷ�������Ѿ���ȡ�ɹ��ˣ����������ֽ��������кܶ౨�ģ���ֻһ��ͷ���ڵ㣬�����յ�һ�������ľ��ȴ���
				short data_id = 0;   //����id
				short data_len = 0;  //���ݳ���
				memcpy(&data_id, m_recv_head_node->m_data, HEAD_ID_LENGTH);
				memcpy(&data_len, m_recv_head_node->m_data + HEAD_ID_LENGTH, HEAD_DATA_LENGTH);

				//�����ֽ���ת������·�ֽ���
				data_id = boost::asio::detail::socket_ops::network_to_host_short(data_id);
				data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);
				std::cout << "data id is " << data_id << std::endl;
				std::cout << "data len is " << data_len << std::endl;

				//�ж�ͷ�������Ƿ�Ϸ�
				if (data_len > max_length)
				{
					std::cout << "invalid data len is " << data_len << std::endl;
					m_server->clear_session(m_uuid);   //ֱ�ӹر�����
					return;
				}

				//��ʼ������Ϣ�壬�ȴ���һ����Ϣ���MsgNode
				m_recv_msg_node = std::make_shared<RecvNode>(data_len, data_id);

				//�ж���Ϣ���Ƿ��������������������ô��Ҫ�����������ݣ�����Ŀǰ�յ������ݻ��浽��Ϣ��ڵ���
				if (bytes_transferred < data_len)
				{
					m_recv_msg_node->append(m_data + copy_len, bytes_transferred);

					//������������
					memset(m_data, 0, max_length);
					m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self_session));
					
					//�����ͷ���Ѿ����������
					m_head_parse_flag = true;

					return;
				}

				//������˵����Ϣͷ���������ˣ�ʣ���δ����������Ҳ����������Ϣ��
				m_recv_msg_node->append(m_data + copy_len, data_len);
				copy_len += data_len;
				bytes_transferred -= data_len;

				std::cout << "receive data:" << m_recv_msg_node->m_data << std::endl;

				//���յ������ݻ���
				send(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len, data_id);

				//������һ�����ģ���Ҫ��ʼ��һ����Ҫ����Ϣͷ���������ȥ�������¿�ʼ
				m_head_parse_flag = false;

				//��Ϣͷ���ڵ���ո���
				m_recv_head_node->clear();

				//����Ѿ������յ����ݶ��������ˣ���ô��Ҫ��������
				if (bytes_transferred <= 0)
				{
					//������������
					memset(m_data, 0, max_length);
					m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self_session));

					return;
				}

				//������˵���Ѿ���������һ����������Ϣ���һ���ʣ�����ݣ���ô��Ҫ����ѭ������
				continue;
			}

			//������˵����Ϣͷ���������ˣ�������Ϣ�廹δ������
			int remain_msg = m_recv_msg_node->m_total_len - m_recv_msg_node->m_cur_len;  //��Ϣ�廹ȱ��������
			
			//ʣ��δ��������ݲ���һ����������Ϣ��ģ���Ҫ��������
			if (bytes_transferred < remain_msg)
			{
				//������յ������ݻ�������
				m_recv_msg_node->append(m_data + copy_len, bytes_transferred);

				//�����������������
				memset(m_data, 0, max_length);
				m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self_session));
				
				return;
			}

			//������˵��ʣ�µ�δ���������������һ����Ϣ��Ĵ�С
			m_recv_msg_node->append(m_data + copy_len, remain_msg);
			copy_len += remain_msg;
			bytes_transferred -= remain_msg;

			std::cout << "receive data:" << m_recv_msg_node->m_data << std::endl;

			//������Ϣ
			short data_id = 0;
			memcpy(&data_id, m_recv_head_node->m_data, HEAD_ID_LENGTH);
			data_id = boost::asio::detail::socket_ops::network_to_host_short(data_id);
			send(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len, data_id);

			//��������һ����������Ϣ��Ҫ���¿�ʼ������Ϣͷ
			m_head_parse_flag = false;

			//m_recv_msg_node����Ҫ�ֶ�������Ϊÿ�δ�����������Ϣͷ��ʱ�򶼻����´���һ��m_recv_msg_node
			m_recv_head_node->clear();

			//����Ѿ�û��ʣ��δ����������ˣ���ô��Ҫ���¿�ʼ��������
			if (bytes_transferred <= 0)
			{
				memset(m_data, 0, max_length);
				m_socket.async_read_some(boost::asio::buffer(m_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self_session));

				return;
			}

			//�ߵ�����˵����������һ����������Ϣ�����ǻ���ʣ��δ��������ݣ���ô��Ҫ����ѭ������
			continue;
		}
	}
	else
	{
		std::cout << "headle read failed, error is " << ec.what() << std::endl;
		
		//�ر�����
		close();

		m_server->clear_session(m_uuid);
	}
}

void Session::handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<Session> self_session)
{
	if (!ec)
	{
		//��������
		std::lock_guard<std::mutex> guard(m_send_queue_lock);

		//д�ص�����������˵��һ��ȫ�����ˣ��򽫷��Ͷ����еĵ�һ��MsgNode�Ƴ�
		m_send_queue.pop();

		if (!m_send_queue.empty())
		{
			//���Ͷ����л�����������Ϣ��Ҫ���ͣ����������
			auto& node = m_send_queue.front();

			boost::asio::async_write(m_socket, boost::asio::buffer(node->m_data, node->m_total_len), std::bind(&Session::handle_write, this, std::placeholders::_1, std::placeholders::_2, self_session));
		}
	}
	else
	{
		std::cout << "handle write failed, error is " << ec.what() << std::endl;

		//�������ر�����
		close();

		//����������ָ�룬�������ֶ��ͷ��ˣ���Ϊ������ָ���Server�����ݽṹ���Ƴ�
		m_server->clear_session(m_uuid);
	}
}