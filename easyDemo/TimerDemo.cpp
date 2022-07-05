#include <iostream>
#include <functional>
#include <thread>
#include "asio.hpp"

#if 0
//Timer1
/////////////////////////////
void synchronously()
{
    asio::io_context io;
    asio::steady_timer t(io, asio::chrono::seconds(5));
    std::cout << "Hello, world!" << std::endl;
    t.wait();
}

//Timer2
/////////////////////////////
void asynchronously()
{
    asio::io_context io;
    asio::steady_timer t(io, asio::chrono::seconds(5));

    t.async_wait([] (const asio::error_code&) { std::cout<<"hwllo world"<<std::endl; });

    io.run();
}

//Timer3
/////////////////////////////
void print(const asio::error_code& e, asio::steady_timer* t, int* count)
{
    if(*count < 5)
    {
        std::cout<<*count<<std::endl;
        ++(*count);

        t->expires_at(t->expiry() + asio::chrono::seconds(1));

        t->async_wait(std::bind(print, std::placeholders::_1, t, count));
    }
}

void bind_asynchronously()
{
    int count = 0;
    asio::io_context io;
    asio::steady_timer t(io, asio::chrono::seconds(1));

    t.async_wait(std::bind(print, std::placeholders::_1, &t, &count));

    io.run();
}

//Timer4
/////////////////////////////
class TimerPrinter
{
public:
    TimerPrinter(asio::io_context& io):m_timer(io, asio::chrono::seconds(1)), m_count(0)
    {
        m_timer.async_wait(std::bind(&TimerPrinter::print, this));
    }

    ~TimerPrinter()
    {
        std::cout<<"finished"<<std::endl;
    }

    void print()
    {
        if(m_count < 5)
        {
            std::cout<<m_count<<std::endl;
            ++m_count;
            m_timer.expires_at(m_timer.expiry() + asio::chrono::seconds(1));
            m_timer.async_wait(std::bind(&TimerPrinter::print, this));
        }
    }

private:
    asio::steady_timer m_timer;
    int m_count;
};

void class_bind_asynchronously()
{
    asio::io_context io;
    TimerPrinter p(io);
    io.run();
}

#endif

//Timer5
/////////////////////////////
class TimerPrinter
{
public:
  TimerPrinter(asio::io_context& io)
    : m_strand(asio::make_strand(io)),
      m_timer1(io, asio::chrono::seconds(1)),
      m_timer2(io, asio::chrono::seconds(1)),
      m_count(0)
  {
    m_timer1.async_wait(asio::bind_executor(m_strand,
          std::bind(&TimerPrinter::print1, this)));

    m_timer2.async_wait(asio::bind_executor(m_strand,
          std::bind(&TimerPrinter::print2, this)));
  }

  ~TimerPrinter()
  {
    std::cout << "Final count is " << m_count << std::endl;
  }

  void print1()
  {
    if (m_count < 10)
    {
      std::cout << "Timer 1: " << m_count << std::endl;
      ++m_count;

      m_timer1.expires_at(m_timer1.expiry() + asio::chrono::seconds(1));

      m_timer1.async_wait(asio::bind_executor(m_strand,
            std::bind(&TimerPrinter::print1, this)));
    }
  }

  void print2()
  {
    if (m_count < 10)
    {
      std::cout << "Timer 2: " << m_count << std::endl;
      ++m_count;

      m_timer2.expires_at(m_timer2.expiry() + asio::chrono::seconds(1));

      m_timer2.async_wait(asio::bind_executor(m_strand,
            std::bind(&TimerPrinter::print2, this)));
    }
  }

private:
  asio::strand<asio::io_context::executor_type> m_strand;
  asio::steady_timer m_timer1;
  asio::steady_timer m_timer2;
  int m_count;
};

void class_bind_multi_asynchronously()
{
    asio::io_context io;
    TimerPrinter p(io);
    //std::thread th(std::bind(static_cast<size_t (asio::io_service::*)()>(&asio::io_service::run), &io));
    std::thread th([&io](){ io.run(); });
    io.run();
    th.join();
}

int main()
{
    //class_bind_asynchronously();
    class_bind_multi_asynchronously();

    return 0;
}