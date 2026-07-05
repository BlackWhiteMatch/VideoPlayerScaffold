#include <jsoncpp/json/json.h>
#include <iostream>
#include <string>

int main(){
    Json::Value val;
    val["name"] = "zhangsan";
    val["age"] = 18;
    val["socre"].append(99);
    val["socre"].append(98);
    val["socre"].append(97.5);

    std::string josnstr = val.toStyledString();
    std::cout << josnstr << std::endl;
    return 0;
}