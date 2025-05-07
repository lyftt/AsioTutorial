#include <iostream>
#include "WebSocketServer.h"
#include <memory>

int main()
{
	std::make_shared<int>(5);
	std::shared_ptr<int> p(new int{0});

	boost::asio::io_context ioc;

	WebSocketServer svr(ioc, 6534);
	svr.start_accept();

	ioc.run();

	return 0;
}