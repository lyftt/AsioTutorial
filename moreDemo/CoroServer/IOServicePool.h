#pragma once
#include "boost/asio.hpp"
#include <memory>
#include <vector>
#include <thread>

class IOServicePool
{
public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::io_context::work;
	using WorkPtr = std::unique_ptr<Work>;

	~IOServicePool();

	/*��ֹ����*/
	IOServicePool(const IOServicePool&) = delete;
	IOServicePool& operator=(const IOServicePool&) = delete;

	/*round-robin��ȡһ��io_context*/
	IOService& get_io_service();

	/*ֹͣ*/
	void stop();

	/*����*/
	static IOServicePool& get_instance();

private:
	IOServicePool(std::size_t size = std::thread::hardware_concurrency());
	std::vector<IOService> m_io_services;
	std::vector<WorkPtr>   m_works;
	std::vector<std::thread> m_threads;
	std::size_t m_next_io_service;
};

