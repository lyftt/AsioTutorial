#include <iostream>
#include <boost/asio.hpp>
#include <memory>

/*创建端点*/
void server_end_point();

/*创建socket*/
void create_tcp_socket(boost::asio::io_context& ioc);

/*创建acceptor（老版本）*/
void create_acceptor_socket(boost::asio::io_context& ioc);

/*创建acceptor（新版本）*/
void create_acceptor_socket_new(boost::asio::io_context& ioc);

/*绑定acceptor和端口号*/
void bind_acceptor_socket(boost::asio::io_context& ioc);

/*接收连接*/
void accept_new_connection(boost::asio::io_context& ioc);

/*向socket中同步写入数据*/
void write_to_socket(boost::asio::ip::tcp::socket& sock, const std::string& buf);

/*从socket中同步读取数据，内部使用read_some()函数*/
std::string read_from_socket(boost::asio::ip::tcp::socket& sock, std::size_t need_size);

/*从socket中同步读取数据，内部使用rceive()函数，会一次性收完，不需要循环每次接收一点*/
std::string read_from_socket_until_all(boost::asio::ip::tcp::socket& sock, std::size_t need_size);

/*从socket中同步读取数据，内部使用read()函数，会一次性收完，不需要循环每次接收一点*/
std::string read_from_socket_until_all_by_read(boost::asio::ip::tcp::socket& sock, std::size_t need_size);


int main()
{
    /*asio的上下文（老版本是io_service），socket 依赖这个asio上下文*/
    boost::asio::io_context ioc;

    accept_new_connection(ioc);

    return 0;
}

void server_end_point()
{
    unsigned short port = 6555;

    /*创建ip地址*/
    //服务器方通常采用这种方式，即any()
    boost::asio::ip::address ip_address = boost::asio::ip::address_v4::any();   //这里没有使用错误码

    boost::asio::ip::tcp::endpoint ep(ip_address, port);

    return;
}

void create_tcp_socket(boost::asio::io_context& ioc)
{
    /*创建一个ipv4协议*/
    boost::asio::ip::tcp protocol = boost::asio::ip::tcp::v4();

    /*创建socket，指定上下文*/
    //也可以这样：
    //boost::asio::ip::tcp::socket sock(ioc, boost::asio::ip::tcp::v4()); 这样就不需要去打开了，内部会自动进行
    boost::asio::ip::tcp::socket sock(ioc);

    boost::system::error_code ec;  //错误码

    /*打开socket，判断是否打开成功*/
    //目前最小版本的asio已经不需要open这个操作了，直接创建socket就行了
    sock.open(protocol, ec);
    if (ec.value() != 0)
    {
        std::cout <<
            "failed to open socket, error code:" << ec.value() << ", message:" << ec.message() << std::endl;
    }

    return;
}

/*老版本的做法*/
//这个函数里的做法是创建acceptor的时候不指定协议，在后面手动open的时候指定协议，且需要判断是否成功
//open以后还需要bind端点才能来时监听
void create_acceptor_socket(boost::asio::io_context& ioc)
{
    boost::asio::ip::tcp::acceptor acptor(ioc);  //这里创建acceptor的时候没有指定协议，后面就需要手动打开

    boost::system::error_code ec;  //错误码

    //老版本才需要这个操作来判断打开是否成功
    acptor.open(boost::asio::ip::tcp::v4(), ec);
    if (ec.value() != 0)
    {
        std::cout <<
            "failed to open acceptor, error code:" << ec.value() << ", message:" << ec.message() << std::endl;
    }

    return;
}

/*新版本做法，很方便*/
//这个做法直接在创建acceptor的时候指定了端点和端口号，内部会直接自动open，且会自动bind，这个函数返回后就已经开始监听了，不需要open也不需要bind
void create_acceptor_socket_new(boost::asio::io_context& ioc)
{
    //新版本的做法简单，指定了ipv4协议，接收所有来自本地6555端口的连接，不需要手动bind
    //这种方式会自动open、自动bind、自动listen，即一口气全做了
    boost::asio::ip::tcp::acceptor acptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 6555));
}

/*老版本做法*/
//这个函数中在创建acceptor的时候，指定了协议，所以会自动open，不需要手动open了
//但是还是需要手动bind端点，acceptor才能开始监听连接
void bind_acceptor_socket(boost::asio::io_context& ioc)
{
    //创建端点
    unsigned short port = 6555;
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), port);   //来自本地所有ip和6555端口的连接

    boost::asio::ip::tcp::acceptor acptor(ioc, ep.protocol());   //创建acceptor的时候如果指定协议，内部会自动打开，不需要手动打开

    boost::system::error_code ec;  //错误码

    //绑定端点到acceptor，还需要手动listen
    acptor.bind(ep, ec);
    if (ec.value() != 0)
    {
        std::cout <<
            "failed to bind acceptor, error code:" << ec.value() << ", message:" << ec.message() << std::endl;
    }

    return;
}

void accept_new_connection(boost::asio::io_context& ioc)
{
    const int BACKLOG_SIZE = 30;
    unsigned short port = 6555;

    //创建端点
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), port);

    try
    {
        //创建acceptor
        boost::asio::ip::tcp::acceptor ac(ioc, ep.protocol());  //会自动打开，但是需要手动bind

        //手动绑定
        ac.bind(ep);    

        //手动监听
        ac.listen(BACKLOG_SIZE);

        //把新接收到的连接交给socket来处理
        std::cout << "listen on " << port << std::endl;
        boost::asio::ip::tcp::socket  sock(ioc);
        ac.accept(sock);

        std::cout << "get one new connection" << std::endl;

        //同步接收数据,如果对端不发数据，则会一值阻塞
        std::string msg = read_from_socket(sock, 5);
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


//同步接收数据
std::string read_from_socket(boost::asio::ip::tcp::socket& sock, std::size_t need_size)
{
    std::size_t total_bytes_read = 0;
    std::unique_ptr<char[]> buf_ptr(new char[need_size]);

    while (total_bytes_read != need_size)
    {
        //同步读取
        //每次读取未必能全部读到，返回值就是读到的字节数
        total_bytes_read += sock.read_some(boost::asio::buffer(buf_ptr.get() + total_bytes_read, need_size - total_bytes_read));;
    }

    return std::string(buf_ptr.get(), need_size);
}

//同步接收数据
std::string read_from_socket_until_all(boost::asio::ip::tcp::socket& sock, std::size_t need_size)
{
    std::unique_ptr<char[]> buf_ptr(new char[need_size]);

    //和send类似，全部接收了才返回，其返回值只会有以下3种可能（有人这么说，但看源码应该是和read_some函数一样）
    //recv_length < 0   发送系统级错误
    //recv_length = 0   对方断开
    //recv_length = need_size  一定是全部接收
    int recv_length = sock.receive(boost::asio::buffer(buf_ptr.get(), need_size));

    return std::string(buf_ptr.get(), need_size);
}

std::string read_from_socket_until_all_by_read(boost::asio::ip::tcp::socket& sock, std::size_t need_size)
{
    std::unique_ptr<char[]> buf_ptr(new char[need_size]);

    //read函数会全部发了才返回，其返回值只会有以下3种可能
    //read_length < 0   发送系统级错误
    //read_length = 0   对方断开
    //readlength = need_size  一定是全部接收
    int recv_length = boost::asio::read(sock, boost::asio::buffer(buf_ptr.get(), need_size));

    return std::string(buf_ptr.get(), need_size);
}