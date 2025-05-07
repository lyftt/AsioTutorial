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
	//给每个io_context绑定一个work
	for (std::size_t i = 0; i < size; ++i)
	{
		//m_works[i].reset(new Work(m_io_services[i]));   //这样也行
		m_works[i] = std::unique_ptr<Work>(new Work(m_io_services[i]));
	}

	//创建多个线程
	for (std::size_t i = 0; i < m_io_services.size(); ++i)
	{
		//直接构造
		m_threads.emplace_back([this, i]() {
			m_io_services[i].run();
		});
	}
}

void IOServicePool::stop()
{
	//释放每一个work
	for (auto& work : m_works)
	{
		//因为仅仅执行work.reset并不能让iocontext从run的状态中退出
		//当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务
		work->get_io_context().stop();   
		work.reset();
	}

	//等待每个线程退出
	for (auto& t : m_threads)
	{
		t.join();
	}
}
