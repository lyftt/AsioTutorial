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

	/*获取线程池共享的io_context*/
	boost::asio::io_context& get_io_context();

	/*停止*/
	void stop();

private:
	IOThreadPool(std::size_t size = std::thread::hardware_concurrency());

	/*内部共享的io_context*/
	boost::asio::io_context m_io_context;

	/*work对象用来保持io_context.run()不退出*/
	std::unique_ptr<boost::asio::io_context::work> m_work;

	/*线程*/
	std::vector<std::thread> m_threads;
};

