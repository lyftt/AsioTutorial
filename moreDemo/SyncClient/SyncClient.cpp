#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <cstring>

const int MAX_LENGTH = 1024;

int main()
{
    try
    {
        //创建上下文
        boost::asio::io_context ioc;

        //创建address和endpoint
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 6555);

        //创建socket
        boost::asio::ip::tcp::socket sock(ioc);

        //error_code，这里初始了
        boost::system::error_code ec = boost::asio::error::host_not_found;
        
        //发起连接
        sock.connect(ep, ec);
        if (ec)
        {
            std::cout << "connect failed, code:" << ec.value() << ", msg:" << ec.message() << std::endl;
            return 0;
        }

        for (;;)
        {
            //连接成功
            std::cout << "Enter msg:";
            char request[MAX_LENGTH] = { 0 };

            //用户输入要发送的信息
            std::cin.getline(request, MAX_LENGTH);
            std::size_t request_len = strlen(request);

            //同步发送
            boost::asio::write(sock, boost::asio::buffer(request, request_len));

            //准备接收消息
            char reply[MAX_LENGTH] = { 0 };
            std::size_t reply_len = boost::asio::read(sock, boost::asio::buffer(reply, request_len));
            std::cout << "reply:";
            std::cout.write(reply, reply_len);
            std::cout << std::endl;
        }
    }
    catch (boost::system::system_error& e)
    {
        std::cout << "exception:" << e.what() << std::endl;
    }

    return 0;
}
