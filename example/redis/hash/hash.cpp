#include <iostream>
#include <chrono>
#include <string>
#include <sw/redis++/connection.h>
#include <sw/redis++/connection_pool.h>
#include <sw/redis++/cxx_utils.h>
#include <sw/redis++/redis.h>
#include <unordered_map>

void ShowHashData(sw::redis::Redis& redis_client, sw::redis::StringView hash_name = "user"){
    std::cout << "\"" << std::string(hash_name.data(), hash_name.size()) << "\"" << ": " << std::endl;
    std::unordered_map<std::string, std::string> current_map;
    redis_client.hgetall("user", std::inserter(current_map, current_map.begin()));
    for(auto& m_pair : current_map){
        std::cout << "\t" << "{" << m_pair.first << ", " << m_pair.second << "}" << std::endl;
    }
}

int main(){
    sw::redis::ConnectionOptions connection_opts{
        .host = "192.168.204.128",
        .password = "123456"
    };

    sw::redis::ConnectionPoolOptions connection_pool_opts{
        .size = 10,
        .connection_idle_time = std::chrono::milliseconds(3600)
    };

    sw::redis::Redis redis_client(connection_opts, connection_pool_opts);

    // 设置哈希表的名字
    redis_client.hset("user", "name", "zhangsan");
    ShowHashData(redis_client);

    std::unordered_map<std::string, std::string> user_map{
        {"age", "18"},
        {"gender", "male"},
        {"hobby", "music"}
    };
    // 从map里面设置剩余的属性
    redis_client.hmset("user", user_map.begin(), user_map.end());
    ShowHashData(redis_client);

    // 年龄加 1岁
    redis_client.hincrby("user", "age", 1);
    ShowHashData(redis_client);

    redis_client.del("user");
    return 0;
}
