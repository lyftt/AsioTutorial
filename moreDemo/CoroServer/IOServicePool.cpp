#include "IOServicePool.h"
#include <iostream>

IOServicePool::~IOServicePool()
{
}

IOServicePool::IOServicePool(std::size_t size):m_io_services(size), m_works(size), m_next_io_service(0)
{
	for (std::size_t i = 0; i < size; ++i)
	{
		//ͨ��work��ά��run����
		m_works[i] = std::unique_ptr<Work>(new Work(m_io_services[i]));   //��ֵ���ƶ�
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
		w->get_io_context().stop();   //��Ҫ��io_contextͣ��
		w.reset();   //ָ���ͷ�
	}

	for (auto& t : m_threads)
	{
		t.join();
	}
}

IOServicePool& IOServicePool::get_instance()
{
	static IOServicePool pool;   //c++11�ᱣ֤�̰߳�ȫ
	return pool;
}
