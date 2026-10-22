#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <sw/redis++/connection.h>
#include <sw/redis++/connection_pool.h>
#include <sw/redis++/redis.h>
#include <vector>

int main(){
    // 连接Redis 服务器
    sw::redis::ConnectionOptions connection_opts{
        .host = "192.168.204.128",
        .password = "123456"
    };

    sw::redis::ConnectionPoolOptions connection_poll_opts{
        .size = 10,
        .connection_idle_time = std::chrono::milliseconds(3600)
    };

    sw::redis::Redis redis_client(connection_opts, connection_poll_opts);

    std::string key{"rank:zset"};
    // zset有序集合操作

    // 添加操作
    // 单个添加
    long long new_members_num = redis_client.zadd(key, "userA", 100.0);
    std::cout << "新增成员数量：" << new_members_num << std::endl;

    // vector添加
    std::vector<std::pair<std::string, double>> members = {
        {"user1", 100.0},
        {"user2", 98.0},
        {"user3", 97.0}
    };
    new_members_num = redis_client.zadd(key, members.begin(), members.end());
    std::cout << "新增成员数量：" << new_members_num << std::endl;

    // 查询操作
    // ZRANGE 升序，0 -1 取全部，不带分数
    std::vector<std::string> res_members;
    redis_client.zrange(key, 0, -1, std::back_inserter(res_members));
    std::cout << "ZRANGE 升序，0 -1 取全部，不带分数:" << std::endl;
    for(auto& e : res_members){
        std::cout << e << " ";
    }
    std::cout << std::endl;

    // ZRANGE + WITHSCORES，返回成员+分数
    std::vector<std::pair<std::string, double>> res_with_score;
    redis_client.zrange(key, 0, -1, std::back_inserter(res_with_score));
    std::cout << "ZRANGE + WITHSCORES，返回成员+分数: " << std::endl;
    for(auto& e : res_with_score){
        std::cout << "{" << e.first << ", " << e.second << "}" << " ";
    }
    std::cout << std::endl;

    // ZREVRANGE 降序（排行榜，从高到低）
    std::vector<std::pair<std::string, double>> res_rev;
    redis_client.zrevrange(key, 0, -1, std::back_inserter(res_rev));
    std::cout << "ZREVRANGE 降序（排行榜，从高到低）: " << std::endl;
    for(auto& e : res_with_score){
        std::cout << "{" << e.first << ", " << e.second << "}" << " ";
    }
    std::cout << std::endl;

    redis_client.del(key);
    std::cout << "key: " << key << ", 删除完毕" << std::endl;
    return 0;
}
