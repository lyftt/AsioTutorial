#include "IOServicePool.h"
#include <iostream>

IOServicePool::~IOServicePool()
{
}

IOServicePool::IOServicePool(std::size_t size):m_io_services(size), m_works(size), m_next_io_service(0)
{
	for (std::size_t i = 0; i < size; ++i)
	{
		//通过work来维持run函数
		m_works[i] = std::unique_ptr<Work>(new Work(m_io_services[i]));   //右值会移动
	}

	for (std::size_t i = 0; i < size; ++i)
	{
		m_threads.emplace_back([this, i]() {
			m_io_services[i].run();
		});
	}
}

IOServicePool::IOService& IOServicePool::get_io_service()
{
	auto& s = m_io_services[m_next_io_service];

	if (m_next_io_service == m_io_services.size())
	{
		m_next_io_service = 0;
	}

	return s;
}

void IOServicePool::stop()
{
	for (auto& w : m_works)
	{
		w->get_io_context().stop();   //需要把io_context停掉
		w.reset();   //指针释放
	}

	for (auto& t : m_threads)
	{
		t.join();
	}
}

IOServicePool& IOServicePool::get_instance()
{
	static IOServicePool pool;   //c++11会保证线程安全
	return pool;
}
