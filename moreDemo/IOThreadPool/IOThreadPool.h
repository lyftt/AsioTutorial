#pragma once
#include "Singleton.h"
#include <thread>
#include <memory>
#include <thread>
#include "boost/asio.hpp"

class IOThreadPool:public Singleton<IOThreadPool>
{
	friend class Singleton<IOThreadPool>;
public:
	~IOThreadPool();
	
	IOThreadPool(const IOThreadPool&) = delete;
	IOThreadPool& operator=(const IOThreadPool&) = delete;

	/*��ȡ�̳߳ع����io_context*/
	boost::asio::io_context& get_io_context();

	/*ֹͣ*/
	void stop();

private:
	IOThreadPool(std::size_t size = std::thread::hardware_concurrency());

	/*�ڲ������io_context*/
	boost::asio::io_context m_io_context;

	/*work������������io_context.run()���˳�*/
	std::unique_ptr<boost::asio::io_context::work> m_work;

	/*�߳�*/
	std::vector<std::thread> m_threads;
};

