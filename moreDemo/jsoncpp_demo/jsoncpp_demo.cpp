#include <iostream>
#include "json/json.h"
#include "json/value.h"
#include "json/reader.h"

int main()
{
    Json::Value root;

    root["id"] = 1001;
    root["data"] = "hello world";

    std::string request = root.toStyledString();   //json对象序列化成字符串
    std::cout << "request is " << request << std::endl;

    Json::Value root2;
    Json::Reader reader;
    reader.parse(request, root2);    //反序列化成json对象

    std::cout << "msg id is " << root2["id"] << " msg is " << root2["data"] << std::endl;

    getchar();
    return 0;
}