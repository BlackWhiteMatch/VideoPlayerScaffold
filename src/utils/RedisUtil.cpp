#include "../../include/VideoPlayerScaffold/utils/RedisUtil.h"
#include <chrono>
#include <iostream>
#include <iterator>
#include <optional>
#include <ratio>
#include <sw/redis++/connection.h>
#include <sw/redis++/connection_pool.h>
#include <sw/redis++/errors.h>
#include <sw/redis++/redis.h>

namespace RedisUtil {
RedisClient::RedisClient(const RedisConfig& config)
    :_config(config)
{
    // 创建 connection_opt
    sw::redis::ConnectionOptions connection_opts{
        .host = _config.host,
        .port = _config.port,
        .user = _config.user,
        .password = _config.password,
        .db = _config.db,
        .connect_timeout = _config.connection_timeout,
        .socket_timeout = _config.socket_timeout
    };

    // 创建线程池配置
    sw::redis::ConnectionPoolOptions connection_pool_opts{
        .size = _config.pool_size,
        .connection_idle_time = _config.pool_idle_interval
    };

    _redis = std::make_unique<sw::redis::Redis>(connection_opts, connection_pool_opts);
}

void RedisClient::track_latency(const char* cmd, const std::string& key, std::chrono::steady_clock::time_point start_time){
    auto end_point = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> cost = end_point - start_time;
    double cost_ms = cost.count();
    if(cost_ms > _config.slow_query_ms){
        std::cerr << "[WARN] [Redis SlowQuery] Cmd: " << cmd
        << " | Key: " << key
        << " | Latency: " << cost_ms << " ms\n";
    }
}

std::optional<std::string> RedisClient::get(const std::string &key){
    auto start = std::chrono::steady_clock::now();
    try{
        auto val = _redis->get(key);
        track_latency("GET", key, start);
        return val;
    }catch(const sw::redis::TimeoutError& e){
        std::cerr << "[ERROR] [Redis Timeout] GET key: " << key << " err: " << e.what() << "\n";
    }catch(const sw::redis::Error& e){
        std::cerr << "[ERROR] [Redis Error] GET key: " << key << " err: " << e.what() << "\n";
    }
    return std::nullopt;
}

bool RedisClient::set(const std::string& key, const std::string& val, std::chrono::milliseconds ttl){
    auto start = std::chrono::steady_clock::now();
    try{
        if(ttl.count() > 0){
            _redis->set(key, val, ttl);
        }else if(ttl.count() == 0){
            _redis->set(key, val);
        }
        track_latency("SET", key, start);
        return true;
    }catch(const sw::redis::TimeoutError& e){
        std::cerr << "[ERROR] [Redis Timeout] SET key: " << key << " err: " << e.what() << "\n";
    }catch(const sw::redis::Error& e){
        std::cerr << "[ERROR] [Redis Error] SET key: " << key << " err: " << e.what() << "\n";
    }
    return false;
}

bool RedisClient::del(const std::string& key){
    auto start = std::chrono::steady_clock::now();
    try{
        auto val = _redis->del(key);
        track_latency("DEL", key, start);
        return val > 0;
    }catch(const sw::redis::TimeoutError& e){
        std::cerr << "[ERROR] [Redis Timeout] DEL key: " << key << " err: " << e.what() << "\n";
    }catch(const sw::redis::Error& e){
        std::cerr << "[ERROR] [Redis Error] DEL key: " << key << " err: " << e.what() << "\n";
    }
    return false;
}

bool RedisClient::exists(const std::string& key){
    auto start = std::chrono::steady_clock::now();
    try{
        auto val = _redis->exists(key);
        track_latency("EXISTS", key, start);
        return val > 0;
    }catch(const sw::redis::TimeoutError& e){
        std::cerr << "[ERROR] [Redis Timeout] EXISTS key: " << key << " err: " << e.what() << "\n";
    }catch(const sw::redis::Error& e){
        std::cerr << "[ERROR] [Redis Error] EXISTS key: " << key << " err: " << e.what() << "\n";
    }
    return false;
}

sw::redis::Redis& RedisClient::raw(){
    return *_redis;
}

}
