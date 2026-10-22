#include <chrono>
#include <iostream>
#include <iterator>
#include <sw/redis++/connection.h>
#include <sw/redis++/connection_pool.h>
#include <sw/redis++/cxx_utils.h>
#include <sw/redis++/redis.h>
#include <sw/redis++/shards.h>
#include <vector>

void ShowList(sw::redis::Redis& redis_client, const sw::redis::StringView& list_name){
    long long list_nums = redis_client.llen(list_name);
    std::vector<std::string> vals;
    redis_client.lrange(list_name, 0, -1, std::back_inserter(vals));
    std::cout << "\"" << std::string(list_name.data(), list_name.size()) << "\"" << ": ";
    for(auto& val : vals){
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

int main(){
    sw::redis::ConnectionOptions connection_opts{
        .host = "192.168.204.128",
        .password = "123456"
    };

    sw::redis::ConnectionPoolOptions connection_poll_opts{
        .size = 10,
        .connection_idle_time = std::chrono::milliseconds(3600)
    };

    sw::redis::Redis redis_client(connection_opts, connection_poll_opts);

    // Redis++的list操作
    redis_client.lpush("skills", "C++");
    ShowList(redis_client, "skills");

    redis_client.rpush("skills", "Python");
    ShowList(redis_client, "skills");

    redis_client.rpush("skills", "Rust");
    ShowList(redis_client, "skills");

    redis_client.lpop("skills");
    ShowList(redis_client, "skills");

    redis_client.rpop("skills");
    ShowList(redis_client, "skills");

    redis_client.del("skills");
    return 0;
}
