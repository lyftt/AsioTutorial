#include <ctime>
#include "asio.hpp"
#include <string>
#include <iostream>

std::string makeDateTime()
{
    time_t now = time(NULL);
    return ctime(&now);
}

int main()
{
    try
    {
        asio::io_context io;
        asio::ip::tcp::acceptor acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 8881));

        for(;;)
        {
            asio::ip::tcp::socket s(io);
            std::cout<<"before accept"<<std::endl;
            acceptor.accept(s);
            std::cout<<"after accept"<<std::endl;

            std::string m = makeDateTime();
            asio::error_code ignored_error;
            asio::write(s, asio::buffer(m), ignored_error);
        }
    }
    catch(std::exception& e)
    {
        std::cout<<"exception:"<<e.what()<<std::endl;
    }
    
    return 0;
}