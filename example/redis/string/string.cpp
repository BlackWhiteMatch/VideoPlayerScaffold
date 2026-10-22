// Redis++ 的String 操作练习
#include <optional>
#include <sw/redis++/connection_pool.h>
#include <sw/redis++/connection.h>
#include <sw/redis++/redis++.h>
#include <sw/redis++/redis.h>
#include <iostream>
#include <chrono>

int main(){
    // 初始化Redis客户端配置
    sw::redis::ConnectionOptions connection_opts{
        .host = "192.168.204.128",
        .password = "123456"
    };
    // 初始化Redis 线程库的配置
    sw::redis::ConnectionPoolOptions connecttion_pool_opts{
        .size = 10,
        .connection_idle_time = std::chrono::milliseconds(3600)
    };

    // 初始化Redis
    sw::redis::Redis redis_client(connection_opts, connecttion_pool_opts);

    redis_client.set("name", "zhangsan");
    auto name = redis_client.get("name");
    if(name){
        std::cout << (*name) << std::endl;
    }

    redis_client.set("age", "18");
    auto age = redis_client.get("age");
    if(age){
        std::cout << (*age) << std::endl;
    }

    // age自加1
    redis_client.incrby("age", 1);
    auto age_added = redis_client.get("age");
    if(age_added){
        std::cout << "age_added: " << (*age_added) << std::endl;
    }

    // age自减1
    redis_client.decrby("age", 1);
    auto age_sub = redis_client.get("age");
    if(age_sub){
        std::cout << "age_sub: " << (*age_sub) << std::endl;
    }

    // 删除键值对
    redis_client.del("name");
    if(redis_client.get("name") == std::nullopt){
        std::cout << "name key deleted!" << std::endl;
    }
    redis_client.del("age");
    if(redis_client.get("age") == std::nullopt){
        std::cout << "age key deleted!" << std::endl;
    }

    return 0;
}
