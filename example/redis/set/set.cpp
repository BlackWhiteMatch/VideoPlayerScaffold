#include <iostream>
#include <iterator>
#include <unordered_set>
#include <chrono>
#include <sw/redis++/connection.h>
#include <sw/redis++/connection_pool.h>
#include <sw/redis++/cxx_utils.h>
#include <sw/redis++/redis.h>

void ShowSet(sw::redis::Redis& redis_client, sw::redis::StringView set_name = "skills"){
    std::unordered_set<std::string> skills;
    redis_client.smembers(set_name, std::inserter(skills, skills.begin()));
    std::cout << "\"" << set_name << "\"" << ": ";
    for(auto& skill : skills){
        std::cout << skill << " ";
    }
    std::cout << std::endl;
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

    // 向集合加入内容
    redis_client.sadd("skills", "C++");
    ShowSet(redis_client);

    std::unordered_set<std::string> skills{"Python", "Rust", "Go"};
    redis_client.sadd("skills", skills.begin(), skills.end());
    ShowSet(redis_client);

    if(redis_client.sismember("skills", "C++")){
        std::cout << "C++ in set" << std::endl;
    }

    redis_client.del("skills");

    return 0;
}
