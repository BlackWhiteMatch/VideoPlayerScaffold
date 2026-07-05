#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <memory>
#include <string>
#include <iostream>

int main(){
    std::string str = R"({"age":18,"name":"zhangsan","score":[98.5,96.5,95.3]})";
    Json::CharReaderBuilder builder;

    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value val;
    std::string errs;
    bool res = reader->parse(str.c_str(), str.c_str() + str.size(), &val, &errs);
    if(res == false){
        std::cout << "Json parse faild: " << errs << std::endl;
    }
    if(!val.isNull() && val["score"].isArray()){
        std::cout << "name: " << val["name"].asString() << std::endl;
        std::cout << "age: " << val["age"].asInt() << std::endl;
        for(int i = 0; i < val["score"].size(); i++){
            std::cout << R"(val["score"])" << "[" << i << "]: " << val["score"][i].asDouble() << std::endl;
        }
    }
    return 0;
}