#include "test.pb.h"
#include <google/protobuf/map_type_handler.h>
#include <iostream>

int main(){
    test::person p1;
    p1.set_name("zhangsan");
    p1.set_age(18);
    p1.set_email("2536877903@qq.com");
    p1.mutable_score()->Add(99.6);
    p1.mutable_score()->Add(95.6);
    
    // map 的使用
    auto* map = p1.mutable_other();
    (*map)[1] = "1";
    (*map)[2] = "2";

    std::string serializeStr = p1.SerializeAsString();
    std::cout << serializeStr << std::endl;
    return 0;
}