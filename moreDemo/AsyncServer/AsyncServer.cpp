#include <iostream>
#include "Session.h"
#include <memory>

struct A:std::enable_shared_from_this<A>
{
    int a;
};

int main()
{
    std::cout << sizeof(A) << std::endl;

    try
    {
        boost::asio::io_context ioc;

        Server svr(ioc, 6534);
        ioc.run();
    }
    catch(boost::system::system_error& e)
    {
        std::cout << "error" << std::endl;
    }

    return 0;
}