#include <iostream>
#include <string>
#include "msg.pb.h"

int main()
{
    Book book;
    
    book.set_name("CPP");
    book.set_pages(100);
    book.set_price(200);

    std::string bookstr;
    book.SerializePartialToString(&bookstr);   //将book序列化并写到字符串中
    std::cout << "serialize str is" << bookstr << std::endl;

    Book book2;
    book2.ParseFromString(bookstr);    //反序列化
    std::cout << "book2 name:" << book2.name() << ", book2 pages:" << book2.pages() << ", book2 price:" << book2.price() << std::endl;

    getchar();

    return 0;
}
