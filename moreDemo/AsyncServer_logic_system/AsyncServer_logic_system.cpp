#include <iostream>
#include "Server.h"

class B {
public:
    int f(int a, int b) { return 5; }
};


template<typename _mem_func_ptr_t, typename _obj_t, typename... _arg_ts>
int fullname_invoke(_mem_func_ptr_t&& mem_func_ptr, _obj_t&& obj, _arg_ts &&... args)
{
    return (std::forward<_obj_t>(obj)->*std::forward<_mem_func_ptr_t>(mem_func_ptr))(std::forward<_arg_ts>(args)...);
}

int main()
{
    B tb;
    auto re = fullname_invoke(&B::f, &tb, 1, 2);
    std::cout << "re:" << re << std::endl;

    /*try
    {
        boost::asio::io_context ioc;

        //使用asio自己的信号处理
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](auto, auto) {
            ioc.stop();
        });

        Server svr(ioc, 6534);
        ioc.run();
    }
    catch (boost::system::system_error& e)
    {
        std::cout << "error" << std::endl;
    }*/

    return 0;
}