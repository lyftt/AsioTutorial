#include <iostream>
#include <memory>
#include <array>
#include <vector>
#include "asio.hpp"

struct Session: public std::enable_shared_from_this<Session>
{
    explicit Session(asio::ip::tcp::socket s):m_socket(std::move(s))
    {

    }

    void Start()
    {
        DoRead();
    }

    void DoRead()
    {
        auto self = shared_from_this();  //获取shared_ptr
        m_socket.async_read_some(asio::buffer(m_data), [this, self](asio::error_code ec, std::size_t length)
        {
            if(!ec)
            {
                std::cout<<"DoRead count:"<<self.use_count()<<std::endl;
                DoWrite(length);
            }
            else
            {
                std::cout<<"read some error"<<std::endl;
            }
        });
    }

    void DoWrite(std::size_t length)
    {
        auto self = shared_from_this();  //获取智能指针
        asio::async_write(m_socket, asio::buffer(m_data, length), [this, self](asio::error_code ec, std::size_t length)
        {
            if(!ec)
            {
                std::cout<<"DoWrite count:"<<self.use_count()<<std::endl;
                DoRead();
            }
            else
            {
                std::cout<<"write error"<<std::endl;
            }
        });
    }

private:
    asio::ip::tcp::socket m_socket;
    std::array<char, 128> m_data;
};

struct Server
{
    //初始化acceptor，关联到io_context
    explicit Server(asio::io_context& ioc, unsigned short port):m_acceptor(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {
        DoAccept();
    }

private:
    void DoAccept()
    {
        //异步
        m_acceptor.async_accept([this](asio::error_code ec, asio::ip::tcp::socket s)
        {
            if(!ec)
            {
                std::make_shared<Session>(std::move(s))->Start();  //使用shared_ptr来管理连接的生命周期
                DoAccept();
            }
            else
            {
                std::cout<<"accept error"<<std::endl;
            }
        });
    }

private:
    asio::ip::tcp::acceptor m_acceptor;
};

int main()
{
    unsigned short port = 6125;
    asio::io_context ioc;
    Server server(ioc, port);

    ioc.run();
    return 0;
}