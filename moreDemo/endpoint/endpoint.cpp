#include <iostream>
#include <boost/asio.hpp>
#include <string>

/*创建端点*/
void client_end_point();

/*发起连接（以ip的方式）*/
void connect_to_server(boost::asio::io_context& ioc);

/*发起连接（以域名的方式）*/
void dns_connect_to_server(boost::asio::io_context& ioc);

/*同步发送一些数据，里面使用的是write_some，未必能一次性发完，需要循环多次发送*/
void write_to_socket(boost::asio::ip::tcp::socket& sock, const std::string& buf);

/*同步发送数据，里面使用的是send，直到发送完才返回*/
void write_to_socket_until_all(boost::asio::ip::tcp::socket& sock, const std::string& buf);

/*同步发送数据，里面使用的是write，直到发送完才返回*/
void write_to_socket_until_all_by_write(boost::asio::ip::tcp::socket& sock, const std::string& buf);

int main()
{
    /*asio的上下文（老版本是io_service），socket 依赖这个asio上下文*/
    boost::asio::io_context ioc;

    connect_to_server(ioc);

    return 0;
}

void client_end_point()
{
    /*ip地址字符串信息和端口信息*/
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port = 6555;
    boost::system::error_code ec;   //错误码,用于出错的情况

    /*转换得到asio的ip地址*/
    //通过字符串来转换得到ip使用的address，是否转换成功，可以通过ec来判断
    //如果是服务段，则可以使用 
    //boost::asio::ip:address ip_address = boost::asio::ip::address_v4::any() 表示任意地址
    boost::asio::ip::address  ip_address = boost::asio::ip::address::from_string(raw_ip_address, ec);   
    if (ec.value() != 0)   //不等于0表示有错误
    {
        std::cout << "failed to parse ip to ip address, error code:" << ec.value() <<", message:" << ec.message() << std::endl;   //ec有错误码和字符串信息
        return;
    }

    /*将asio的ip地址和端口号组合起来，形成endpoint，endpoint才可以真正用于tcp连接*/
    boost::asio::ip::tcp::endpoint tp(ip_address, port);

    return;
}

void connect_to_server(boost::asio::io_context& ioc)
{
    //服务器的地址
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port = 6555;

    try
    {
        //创建目标端点
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(raw_ip_address), port);

        //创建socket
        boost::asio::ip::tcp::socket sock(ioc, ep.protocol());   //端点转换成协议，默认其实就是ipv4

        //连接服务器(同步方式)，就是直接去连接代表服务器的端点
        //这里是直接连接ip地址，而不是域名
        sock.connect(ep);

        //同步发送数据
        write_to_socket(sock, "nihao");
    }
    catch (boost::system::system_error& e)  //与error_code存在区别，一个是错误码，一个是异常，是Asio中两种处理错误的方式
    {
        std::cout << "connect failed, err code:" << e.code() << ", msg:" << e.what() <<", value:" << e.code().value() << std::endl;
    }

    return;
}

//这种用的比较少
void dns_connect_to_server(boost::asio::io_context& ioc)
{
    //解析域名
    std::string host = "www.baidu.com";
    std::string port = "8080";

    boost::asio::ip::tcp::resolver::query q(host, port, boost::asio::ip::tcp::resolver::query::numeric_service);   //查询服务

    boost::asio::ip::tcp::resolver resolv(ioc);   //域名解析器

    try
    {
        //query和resolver配合使用，resolve函数来解析查询服务
        boost::asio::ip::tcp::resolver::iterator it = resolv.resolve(q);    //会返回一个迭代器，是能解析到的所有的ip，可能不只一个

        //创建socket
        boost::asio::ip::tcp::socket sock(ioc);
        
        //发起连接，使用全局的connect函数，这里没法直接用socket的connect函数
        boost::asio::connect(sock, it);

    }
    catch (boost::system::system_error& e)
    {
        std::cout << "connect failed, err code:" << e.code() << ", msg:" << e.what() << ", value:" << e.code().value() << std::endl;
    }
}


//同步发送数据
void write_to_socket(boost::asio::ip::tcp::socket& sock, const std::string& buf)
{
    std::size_t total_bytes_written = 0;

    //循环发送
    while (total_bytes_written != buf.length())
    {
        //write_some()是同步发送
        //wrtie_some()函数会返回每次写入的字节数
        //所以发送的时候给buffer()函数的地址还需要加上已发送的字节数偏移
        total_bytes_written += sock.write_some(boost::asio::buffer(buf.c_str() + total_bytes_written, buf.length() - total_bytes_written));
    }
}

//同步发送数据
void write_to_socket_until_all(boost::asio::ip::tcp::socket& sock, const std::string& buf)
{
    //send函数会全部发了才返回，其返回值只会有以下3种可能（有人这么说，但看源码应该是和write_some函数一样）
    //send_length < 0   发送系统级错误
    //send_length = 0   对方断开
    //send_length = buf.length()  一定是全部发送
    int send_length = sock.send(boost::asio::buffer(buf.c_str(), buf.length()));
    if (send_length <= 0)
    {
        std::cout << "error send" << std::endl;
    }
}

//同步发送数据
void write_to_socket_until_all_by_write(boost::asio::ip::tcp::socket& sock, const std::string& buf)
{
    //write函数会全部发了才返回，其返回值只会有以下3种可能
    //write_length < 0   发送系统级错误
    //write_length = 0   对方断开
    //write_length = buf.length()  一定是全部发送
    int send_length = boost::asio::write(sock, boost::asio::buffer(buf.c_str(), buf.length()));

    if (send_length <= 0)
    {
        std::cout << "error write" << std::endl;
    }
}
