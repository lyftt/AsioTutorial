#pragma once
#include <mutex>
#include <memory>
#include <iostream>


//单例基类
template<typename T>
class Singleton
{
protected:
	Singleton() = default;
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator=(const Singleton<T>&) = delete;

	static std::shared_ptr<T> m_instance;   //静态变量
	
public:
	~Singleton()
	{
		std::cout << "singleton destruct" << std::endl;
	}

	static std::shared_ptr<T> get_instance()
	{
		static std::once_flag s_flag;

		//只会初始化1次
		std::call_once(s_flag, [&]() {
			m_instance = std::make_shared<T>();
		});
	}
};

template<typename T>
std::shared_ptr<T> Singleton<T>::m_instance = nullptr;



