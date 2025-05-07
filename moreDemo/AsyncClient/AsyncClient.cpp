#include <iostream>
#include <boost/asio.hpp>

int main()
{
	try
	{
		boost::asio::io_context ioc;

		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 6534);

		boost::asio::ip::tcp::socket sock(ioc);

		boost::system::error_code ec = boost::asio::error::host_not_found;

		sock.connect(ep, ec);

		if (ec)
		{
			std::cout << "connect error" << std::endl;
			return 0;
		}

		for (;;)
		{
			std::cout << "Enter:";
			char request[1024] = { 0 };

			std::cin.getline(request, 1024);
			std::size_t request_len = strlen(request);

			boost::asio::write(sock, boost::asio::buffer(request, request_len));

			char reply[1024] = {0};
			std::size_t reply_len = boost::asio::read(sock, boost::asio::buffer(reply, request_len));
			std::cout << "reply:";
			std::cout.write(reply, reply_len);
			std::cout << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "ececption:" << e.what() << std::endl;
	}

	return 0;
}