#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <set>
#include <thread>

const int MAX_LENGTH = 1024;
using socket_ptr = std::shared_ptr<boost::asio::ip::tcp::socket>;
using thread_ptr = std::shared_ptr<std::thread>;

//每来一个连接会创建一个线程
std::set<thread_ptr> thread_set;

/*服务器处理函数*/
void session(socket_ptr sock)
{
	for (;;)
	{
		char data[MAX_LENGTH] = { 0 };
		boost::system::error_code ec;

		//只能read_some，因为无法知道客户端会发过来多少，无法使用asio::read
		std::size_t length = sock->read_some(boost::asio::buffer(data, MAX_LENGTH), ec);   

		//eof表示对端关闭
		if(ec == boost::asio::error::eof)
		{
			std::cout << "peer closed" << std::endl;
			break;
		}
		//其他错误就比较严重了
		else if(ec)
		{
			throw boost::system::system_error(ec);   //使用error_code来初始化system_error异常
		}

		//未发生错误
		std::cout << "receive from " << sock->remote_endpoint().address().to_string() << std::endl;
		std::cout << "receive msg:" << data << std::endl;

		//将数据回传对方
		boost::asio::write(*sock, boost::asio::buffer(data, length));
	}
}

/*服务器函数*/
void server(boost::asio::io_context& ioc, unsigned port)
{
	boost::asio::ip::tcp::acceptor ac(ioc,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

	for (;;)
	{
		//接收连接
		socket_ptr sock_ptr(new boost::asio::ip::tcp::socket(ioc));
		ac.accept(*sock_ptr);

		//新建线程接管通讯
		auto th = std::make_shared<std::thread>(session, sock_ptr);

		//防止线程析构
		thread_set.insert(th);
	}
}

int main()
{
	try
	{
		//创建上下文
		boost::asio::io_context ioc;

		//运行服务器
		server(ioc, 6555);

		//等待所有子线程退出
		for (auto& t : thread_set)
		{
			t->join();
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "exception:" << e.what() << std::endl;
	}

    return   0;
}
