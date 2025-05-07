#include "boost/asio.hpp"
#include <iostream>

int main() {
    // 创建I/O上下文
    boost::asio::io_context io;

    // 创建一个steady_timer对象，绑定到I/O上下文
    boost::asio::steady_timer timer(io);

    // 设置定时器在5秒后超时
    timer.expires_after(std::chrono::seconds(5));

    // 绑定回调函数，当定时器超时时执行
    timer.async_wait([&]() {
        std::cout << "5秒已到，定时器触发！" << std::endl;
        });

    // 启动I/O上下文的事件循环
    io.run();

    return 0;
}