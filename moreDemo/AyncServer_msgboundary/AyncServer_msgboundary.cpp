#include <iostream>
#include "Server.h"

int main()
{
    try
    {
        boost::asio::io_context ioc;

        Server svr(ioc, 6534);
        ioc.run();
    }
    catch (boost::system::system_error& e)
    {
        std::cout << "error" << std::endl;
    }

    return 0;
}