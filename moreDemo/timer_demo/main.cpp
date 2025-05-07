#include "boost/asio.hpp"
#include <iostream>

int main() {
    // ����I/O������
    boost::asio::io_context io;

    // ����һ��steady_timer���󣬰󶨵�I/O������
    boost::asio::steady_timer timer(io);

    // ���ö�ʱ����5���ʱ
    timer.expires_after(std::chrono::seconds(5));

    // �󶨻ص�����������ʱ����ʱʱִ��
    timer.async_wait([&]() {
        std::cout << "5���ѵ�����ʱ��������" << std::endl;
        });

    // ����I/O�����ĵ��¼�ѭ��
    io.run();

    return 0;
}