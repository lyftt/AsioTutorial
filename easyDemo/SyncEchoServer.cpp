#include <iostream>
#include <array>
#include <vector>
#include "asio.hpp"

void Session(asio::ip::tcp::socket s)
{
    try
    {
        while(true)
        {
            std::array<char, 128> data;
            asio::error_code ec;

            std::size_t len = s.read_some(asio::buffer(data), ec);

            if(ec == asio::error::eof)
            {
                std::cout<<"peer connection break"<<std::endl;
                break;
            }
            else if(ec)
            {
                std::cout<<"other error"<<std::endl;
                throw "other error";
            }

            asio::write(s, asio::buffer(data, len));
        }
    }
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
    }
}

int main()
{
    asio::io_context ioc;

    asio::ip::tcp::acceptor acceptor(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 6537));

    try
    {
        while(true)
        {
            Session(acceptor.accept());
        }
    }
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
    }

    return 0;
}