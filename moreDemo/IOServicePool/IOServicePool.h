#pragma once
#include "boost/asio.hpp"
#include "Singleton.h"
#include <vector>
#include <thread>

/*单例模式*/
class IOServicePool: public Singleton<IOServicePool>
{
	friend Singleton<IOServicePool>;

public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::io_context::work;
	using WorkPtr = std::unique_ptr<Work>;  //只能移动

	~IOServicePool();
	IOServicePool(const IOServicePool&) = delete;
	IOServicePool& operator=(const IOServicePool&) = delete;

	/*round-robin方式返回一个io_context*/
	boost::asio::io_context& get_io_service();

	/**/
	void stop();

private:
	IOServicePool(std::size_t size = std::thread::hardware_concurrency());
	std::vector<IOService>  m_io_services;
	std::vector<WorkPtr> m_works;
	std::vector<std::thread> m_threads;
	std::size_t m_next_io_service;
};

