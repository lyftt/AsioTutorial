#include <iostream>
#include <memory>
#include <functional>
#include "asio.hpp"

void connection_handler(const asio::error_code& ec, std::shared_ptr<asio::ip::tcp::socket> ps, asio::ip::tcp::endpoint ep)
{
    if(!ec)
    {
        std::cout<<"connection successful"<<std::endl;
    }else
    {
        std::cout<<"connection failed, retry"<<std::endl;
        ps->async_connect(ep, std::bind(connection_handler, std::placeholders::_1, ps, ep));
    }
}

int main()
{
    asio::io_context ioc;
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), 6125);

    std::shared_ptr<asio::ip::tcp::socket> ps(new asio::ip::tcp::socket(ioc));
    ps->async_connect(ep, std::bind(connection_handler, std::placeholders::_1, ps, ep));

    ioc.run();

    return 0;
}