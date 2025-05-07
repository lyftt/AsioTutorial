#include "IOThreadPool.h"

IOThreadPool::IOThreadPool(std::size_t size):m_work(new boost::asio::io_context::work(m_io_context))
{
	/*�����߳�*/
	for (std::size_t i = 0; i < size; ++i)
	{
		m_threads.emplace_back([this]() {
			m_io_context.run();   //ÿ���߳��ﶼ�����io_context.run()
		});
	}
}

IOThreadPool::~IOThreadPool()
{
}

boost::asio::io_context& IOThreadPool::get_io_context()
{
	return m_io_context;
}

void IOThreadPool::stop()
{
	m_io_context.run();   //�������Ҫ
	m_work.reset();

	/*���������߳�*/
	for (auto& t : m_threads)
	{
		t.join();
	}
}
