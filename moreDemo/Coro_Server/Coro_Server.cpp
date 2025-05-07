#include <iostream>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio.hpp>

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;

namespace this_coro = boost::asio::this_coro;   //当前协程的执行环境

/*
* 每个连接的数据处理协程
* 
*/
awaitable<void> echo(boost::asio::ip::tcp::socket sock)
{
	//可能有异常
	try
	{
		char data[1024] = { 0 };  //接收数据

		for (;;)
		{
			//co_await表示会阻塞并释放使用权，让调度器去调度其他协程
			//use_awaitable告诉asio我们在使用协程，让async_read_some变成可等待的，可以阻塞
			std::size_t n = co_await sock.async_read_some(boost::asio::buffer(data, sizeof(data)), use_awaitable);

			//协程恢复执行的时候，表示读到数据了，那么需要再将数据发送出去
			//co_await表示会阻塞并释放使用权，让调度器去调度其他协程
			//use_awaitable告诉asio我们在使用协程，让async_write变成可等待的，可以阻塞，这样协程会在这里等待直到发送完，等协程再次被调度的时候数据已经被发送完了
			co_await boost::asio::async_write(sock, boost::asio::buffer(data, n), use_awaitable);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "e:" << e.what() << std::endl;
	}
}

/*
* 监听连接
* 
*/
awaitable<void> listener()
{
	//协程的调度器，co_await表示异步查询调度器，如果查不到就会挂起并把使用权交给其他协程
	//调度器会将协程调度
	auto executor = co_await this_coro::executor;   

	//获取到调度器之后就开始接收连接
	boost::asio::ip::tcp::acceptor acp(executor, {boost::asio::ip::tcp::v4(), 6543});

	//协程最大的好处就是将异步代码写成同步的
	for (;;)
	{
		//co_await会阻塞并释放使用权，让调度器去调度其他协程
		//use_awaitable用来告诉asio我们在使用协程，让async_accept变成可等待的
		boost::asio::ip::tcp::socket sock = co_await acp.async_accept(use_awaitable);

		//协程被唤醒后，要再启动一个协程来处理新建立的连接
		co_spawn(executor, echo(std::move(sock)), detached);
	}
}

int main()
{
	try
	{
		boost::asio::io_context ioc;   //上下文，或者叫调度器
		
		boost::asio::signal_set sigs(ioc, SIGINT, SIGTERM);
		sigs.async_wait([&ioc](auto, auto) {
			ioc.stop();
		});

		//启动一个协程
		//第一个参数是调度器
		//第二个参数是要执行的协程
		//第三个参数是让协程独立运行
		co_spawn(ioc, listener(), detached);

		ioc.run();//如果没有任何异步事件，那么会直接退出
	}
	catch (const std::exception& e)
	{
		std::cout << "e:" << e.what() << std::endl;
	}

    return 0;
}