#include <iostream>
#include "Server.h"
#include "IOThreadPool.h"

int main()
{
    try
    {
        //建立连接的socket使用IOServicePool中的io_context
        auto pool = IOThreadPool::get_instance();

        //acceptor还是使用这个独立的io_context
        boost::asio::io_context ioc;

        //使用asio自己的信号处理
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([pool, &ioc](auto, auto) {
            ioc.stop();
            pool->stop();
            });

        Server svr(pool->get_io_context(), 6534);   //使用线程池公用的io_context

        ioc.run();
    }
    catch (boost::system::system_error& e)
    {
        std::cout << "error" << std::endl;
    }

    return 0;
}