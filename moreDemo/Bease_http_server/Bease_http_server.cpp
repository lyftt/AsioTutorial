#include <iostream>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <memory>
#include "boost/beast.hpp"
#include "boost/asio.hpp"
#include "json/json.h"
#include "json/value.h"
#include "json/reader.h"

/*统计请求数量*/
std::size_t request_count()
{
    static std::size_t count = 0;
    return ++count;
}

/*获取当前时间戳*/
std::time_t now()
{
    return std::time(0);
}

/*http连接对象*/
class HttpConnection :public std::enable_shared_from_this<HttpConnection>
{
public:
    HttpConnection(boost::asio::ip::tcp::socket socket):m_socket(std::move(socket))   //socket对象不能复制，只能移动
    {

    }

    /*启动处理，包括读取、处理、返回*/
    void start()
    {
        read_request();

        check_deadline();
    }

private:
    /*socket*/
    boost::asio::ip::tcp::socket m_socket;

    /*缓存buffer*/
    boost::beast::flat_buffer m_buffer{8192};

    /*请求*/
    boost::beast::http::request<boost::beast::http::dynamic_body> m_request;

    /*响应*/
    boost::beast::http::response<boost::beast::http::dynamic_body> m_response;

    /*定时器*/
    boost::asio::steady_timer m_deadline{ m_socket.get_executor(), std::chrono::seconds(60)};  //1分钟

private:
    /*读请求*/
    void read_request()
    {
        auto self = shared_from_this();

        //异步读
        boost::beast::http::async_read(m_socket, m_buffer, m_request, [self](boost::beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);  //不使用的参数忽略

            if(!ec)
            {
                self->process_request();
            }
        });
    }

    /*判断定时器，超时则关闭socket*/
    void check_deadline()
    {
        auto self = shared_from_this();

        //传this会有风险，可能超时回调函数调用的时候HttpConnection已经释放了,this已经失效了
        m_deadline.async_wait([self](boost::system::error_code ec) {
            if (!ec)
            {
                self->m_socket.close();   //这里直接关闭，而不是shutdown
            }
        });
    }

    /**/
    void process_request()
    {
        m_response.version(m_request.version());
        m_response.keep_alive(false);
        
        switch (m_request.method())
        {
            case boost::beast::http::verb::get:
                m_response.result(boost::beast::http::status::ok);   //状态码
                m_response.set(boost::beast::http::field::server, "Beast");  //请求头部
                create_response();
                break;

            case boost::beast::http::verb::post:
                m_response.result(boost::beast::http::status::ok);   //状态码
                m_response.set(boost::beast::http::field::server, "Beast");  //请求头部
                create_post_response();
                break;

            default:
                m_response.result(boost::beast::http::status::bad_request);   //状态码
                m_response.set(boost::beast::http::field::content_type, "text/plain");  //请求头部
                boost::beast::ostream(m_response.body()) << "Invalid request-method:" << std::string(m_request.method_string());
                break;
        }

        write_response();
    }

    /*get请求的响应*/
    void create_response()
    {
        if (m_request.target() == "/count")
        {
            m_response.set(boost::beast::http::field::content_type, "text/html"); //请求头部
            boost::beast::ostream(m_response.body())
                << "<html>\n"
                << "<head><title>Request count</title></head>\n"
                << "<body>\n"
                << "<h1>Request count</h1>\n"
                << "<p>There have been "
                << request_count()
                << " requests so far.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else if (m_request.target() == "/time")
        {
            m_response.set(boost::beast::http::field::content_type, "text/html"); //请求头部
            boost::beast::ostream(m_response.body())
                << "<html>\n"
                << "<head><title>Current time</title></head>\n"
                << "<body>\n"
                << "<h1>Current time</h1>\n"
                << "<p>The current time is "
                << now()
                << " seconds since the epoch.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else
        {
            m_response.result(boost::beast::http::status::not_found);
            m_response.set(boost::beast::http::field::content_type, "text/plain");  //纯文本返回
            boost::beast::ostream(m_response.body()) << "not FOUND\r\n";
        }
    }

    /*发送响应*/
    void write_response()
    {
        auto self = shared_from_this();

        m_response.content_length(m_response.body().size());   //设置body长度

        boost::beast::http::async_write(m_socket, m_response, [self](boost::beast::error_code ec, std::size_t) {
            self->m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);   //关闭发送通道，但是还是可以接收，尽量不要服务器主动关闭连接，这样会有大量TIME_WAIT
            self->m_deadline.cancel();  //定时器取消掉
        });
    }

    /*post请求的响应*/
    void create_post_response()
    {
        if (m_request.target() == "/email")
        {
            auto& body = m_request.body();
            auto body_str = boost::beast::buffers_to_string(body.data());
            std::cout << "receive body is " << body_str << std::endl;

            m_response.set(boost::beast::http::field::content_type, "text/json");

            Json::Value root;
            Json::Reader reader;
            Json::Value src_root;

            bool isok = reader.parse(body_str, src_root);
            if (!isok)
            {
                root["error"] = 1001;
                std::string jsonstr = root.toStyledString();
                boost::beast::ostream(m_response.body()) << jsonstr;
                return;
            }

            auto email = src_root["email"].asString();
            root["error"] = 0;
            root["email"] = src_root["email"];
            root["msg"] = "recv email post success";

            std::string jsonstr = root.toStyledString();
            boost::beast::ostream(m_response.body()) << jsonstr;
        }
        else
        {
            m_response.result(boost::beast::http::status::not_found);
            m_response.set(boost::beast::http::field::content_type, "text/plain");  //纯文本返回
            boost::beast::ostream(m_response.body()) << "not FOUND\r\n";
        }
    }
};

void http_server(boost::asio::ip::tcp::acceptor& acp, boost::asio::ip::tcp::socket& socket)
{
    acp.async_accept(socket, [&](boost::system::error_code ec) {
        if (!ec)
        {
            std::make_shared<HttpConnection>(std::move(socket))->start();
        }

        http_server(acp, socket);  //继续监听请求
    });
}

int main()
{
    try
    {
        //auto const address = boost::asio::ip::make_address("127.0.0.1");
        unsigned short port = static_cast<unsigned short>(8080);
        boost::asio::io_context ioc;

        boost::asio::ip::tcp::acceptor acp{ ioc, {boost::asio::ip::tcp::v4(), port} };
        boost::asio::ip::tcp::socket sock{ioc};

        http_server(acp, sock);

        ioc.run();
    }
    catch (const std::exception& e)
    {
        std::cout << "exception:" << e.what() << std::endl;
    }

    return 0;
}
