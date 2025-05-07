#include "IOServicePool.h"

IOServicePool::~IOServicePool()
{
}

boost::asio::io_context& IOServicePool::get_io_service()
{
	auto& s = m_io_services[m_next_io_service++];
	if (m_next_io_service == m_io_services.size())
	{
		m_next_io_service = 0;
	}

	return s;
}

IOServicePool::IOServicePool(std::size_t size):m_io_services(size), m_works(size), m_next_io_service(0)
{
	//��ÿ��io_context��һ��work
	for (std::size_t i = 0; i < size; ++i)
	{
		//m_works[i].reset(new Work(m_io_services[i]));   //����Ҳ��
		m_works[i] = std::unique_ptr<Work>(new Work(m_io_services[i]));
	}

	//��������߳�
	for (std::size_t i = 0; i < m_io_services.size(); ++i)
	{
		//ֱ�ӹ���
		m_threads.emplace_back([this, i]() {
			m_io_services[i].run();
		});
	}
}

void IOServicePool::stop()
{
	//�ͷ�ÿһ��work
	for (auto& work : m_works)
	{
		//��Ϊ����ִ��work.reset��������iocontext��run��״̬���˳�
		//��iocontext�Ѿ����˶���д�ļ����¼��󣬻���Ҫ�ֶ�stop�÷���
		work->get_io_context().stop();   
		work.reset();
	}

	//�ȴ�ÿ���߳��˳�
	for (auto& t : m_threads)
	{
		t.join();
	}
}
