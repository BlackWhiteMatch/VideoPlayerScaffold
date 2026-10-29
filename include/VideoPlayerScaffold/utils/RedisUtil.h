#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <optional>
#include <sw/redis++/redis++.h>
#include <sw/redis++/redis.h>
#include <sw/redis++/shards.h>

namespace RedisUtil {
// Redis 配置结构体
struct RedisConfig{
    // 用户配置
    std::string host = "127.0.0.1";
    std::string user = "default";
    std::string password;
    int port = 6379;
    int db = 0;
    double slow_query_ms = 20.0;

    // 超时配置
    std::chrono::milliseconds connection_timeout{200};
    std::chrono::milliseconds socket_timeout{500}; // 读写超时

    // 连接池配置
    size_t pool_size = 1;
    std::chrono::milliseconds pool_idle_interval{3600};
};

class RedisClient {
public:
    explicit RedisClient(const RedisConfig& config);
    ~RedisClient() = default;

    // 禁止拷贝，可移动
    RedisClient(const RedisClient&) = delete;
    RedisClient& operator=(const RedisClient&) = delete;
    RedisClient(RedisClient&&) noexcept = default;
    RedisClient& operator=(RedisClient&&) noexcept = default;

    std::optional<std::string> get(const std::string& key);
    bool set(const std::string& key, const std::string& val, std::chrono::milliseconds ttl = std::chrono::milliseconds(0));
    bool del(const std::string& key);
    bool exists(const std::string& key);

    sw::redis::Redis& raw();

private:
    void track_latency(const char* cmd, const std::string& key, std::chrono::steady_clock::time_point start_time);

    RedisConfig _config;
    std::unique_ptr<sw::redis::Redis> _redis;
};

}
