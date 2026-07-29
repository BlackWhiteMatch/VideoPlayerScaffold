#include "../../include/VideoPlayerScaffold/utils/JsonUtil.h"
#include <iostream>
#include <jsoncpp/json/value.h>
#include <optional>

void serilize_test(const Json::Value& val){
    std::optional<std::string> outputString = JsonUtil::JSON::Serialize(val);
    std::cout << "Serilalize: " << outputString.value_or("") << std::endl;
}

void unserilize_test(const std::string& str){
    std::optional<Json::Value> outputValue = JsonUtil::JSON::UnSerialize(str);
    Json::Value val = outputValue.value();
    if(!outputValue->isNull()){
        std::cout << "name: " << val["name"].asString() << std::endl;
        std::cout << "age: " << val["age"].asInt() << std::endl;
        for(int i = 0; i < val["score"].size(); i++){
            std::cout << R"(val["score"])" << "[" << i << "]: " << val["score"][i].asDouble() << std::endl;
        }
    }
}

int main(){
    Json::Value val;
    val["name"] = "zhangsan";
    val["age"] = 18;
    val["socre"].append(99);
    val["socre"].append(98);
    val["socre"].append(97.5);
    serilize_test(val);

    std::string str = R"({"age":18,"name":"zhangsan","score":[98.5,96.5,95.3]})";
    unserilize_test(str);
    return 0;
}