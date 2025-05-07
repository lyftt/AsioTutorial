#include <iostream>
#include "boost/asio.hpp"
#include "Server.h"
#include "IOServicePool.h"
#include <csignal>
#include <mutex>
#include <thread>

/*
* 同样采用 one loop per thread 的IOServicePool
* 
*/
int main()
{
	try
	{
		//io_context池
		auto& pool = IOServicePool::get_instance();

		boost::asio::io_context ioc;

		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc, &pool](auto, auto) {
			ioc.stop();
			pool.stop();
		});

		//Server独立使用一个ioc进行监听连接，和IOServicePool区分开
		//IOServicePool专门用于处理连接的收发
		Server svr(ioc, 6543);

		ioc.run();
	}
	catch (const std::exception& e)
	{
		std::cout << "error:" << e.what() << std::endl;
	}
}