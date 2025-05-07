#include "Server.h"
#include "Session.h"

Server::Server(boost::asio::io_context& ioc, short port) :m_ioc(ioc), m_ac(m_ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	start_accept();
}

void Server::start_accept()
{
	//创建原始指针，下面要改成使用智能指针
	//Session* session = new Session(m_ioc);
	//创建智能指针
	std::shared_ptr<Session> session = std::make_shared<Session>(m_ioc, this);

	//回调函数只能有一个参数，asio定义的，所以这里需要绑定下第一个参数
	//这里std::bind会生成一个可调用对象，这个对象内部会拷贝一份session这个智能指针，这会让引用计数+1
	m_ac.async_accept(session->socket(), std::bind(&Server::handle_accept, this, session, std::placeholders::_1));
}

void Server::handle_accept(std::shared_ptr<Session> session, const boost::system::error_code& ec)
{
	if (ec)
	{
		std::cout << "handle accept error" << std::endl;

		//释放掉Session，改成智能指针之后就不能使用delete了
		//delete this;

		//如果出问题，那么什么都不用干，不需要插入m_sessions，session的生命周期在handle_accept结束后就会结束
	}
	else
	{
		//开启session会话，收发数据
		session->start();

		//这里必须要有这一步，如果没有这一步，那么session就会最后一个引用计数了，handle_accept函数调用完就会销毁了，
		//后面设置读回调啥的都是有问题的
		m_sessions[session->uuid()] = session;
	}

	//处理完新的连接后，继续接收新的连接
	start_accept();
}

void Server::clear_session(const std::string& uuid)
{
	m_sessions.erase(uuid);
}
