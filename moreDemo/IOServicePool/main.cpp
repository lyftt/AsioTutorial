#include <iostream>
#include <memory>
#include "Server.h"
#include "IOServicePool.h"

int main()
{
    try
    {
        //建立连接的socket使用IOServicePool中的io_context
        auto pool = IOServicePool::get_instance();

        //acceptor还是使用这个独立的io_context
        boost::asio::io_context ioc;

        //测试代码
		auto ex1 = ioc.get_executor();
        ex1.post([]() { std::cout << "a" << std::endl; }, std::allocator<void>());

        boost::asio::post([]() {std::cout << "a" << std::endl; });

        //使用asio自己的信号处理
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([pool, &ioc](auto, auto) {
            ioc.stop();
            pool->stop();
            });

        Server svr(ioc, 6534);
        ioc.run();
    }
    catch (boost::system::system_error& e)
    {
        std::cout << "error" << std::endl;
    }

    return 0;
}