#include "IOThreadPool.h"

IOThreadPool::IOThreadPool(std::size_t size):m_work(new boost::asio::io_context::work(m_io_context))
{
	/*创建线程*/
	for (std::size_t i = 0; i < size; ++i)
	{
		m_threads.emplace_back([this]() {
			m_io_context.run();   //每个线程里都会调用io_context.run()
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
	m_io_context.run();   //这个必须要
	m_work.reset();

	/*回收所有线程*/
	for (auto& t : m_threads)
	{
		t.join();
	}
}
