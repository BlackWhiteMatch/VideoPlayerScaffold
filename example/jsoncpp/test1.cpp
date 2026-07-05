#include <jsoncpp/json/json.h>
#include <iostream>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <memory>
#include <sstream>

int main(){
    Json::Value val;
    val["name"] = "zhangsan";
    val["age"] = "18";
    val["sorce"].append(98.5);
    val["sorce"].append(96.6);
    val["sorce"].append(95.3);

    Json::StreamWriterBuilder builder;
    // 制表符设置为空
    builder.settings_["indentation"] = "";

    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::stringstream ss;
    writer->write(val, &ss);

    std::cout << ss.str() << std::endl;
    return 0;
}