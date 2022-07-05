#include <iostream>
#include <array>
#include <unistd.h>
#include "asio.hpp"

int main()
{
    try
    {
        asio::io_context io;
        asio::ip::tcp::resolver resolver(io);
        asio::ip::tcp::resolver::results_type e = resolver.resolve(asio::ip::tcp::v4(), "127.0.0.1", "8881");
        //asio::ip::tcp::endpoint e(asio::ip::tcp::v4(), 8881);

        asio::ip::tcp::socket s(io);
        asio::connect(s, e);

        for(;;)
        {
            asio::error_code err;
            std::array<char, 128> b;

            size_t len = s.read_some(asio::buffer(b), err);
            std::cout<<"recv "<<len<<" bytes"<<std::endl;

            if(err == asio::error::eof)
            {
                break;
            }
            else if(err)
            {
                throw "other error";
            }

            std::cout.write(b.data(), len);
            sleep(1);
        }
    }catch(std::exception& e)
    {
        std::cout<<"exception:"<<e.what()<<std::endl;
    }   

    return 0;
}