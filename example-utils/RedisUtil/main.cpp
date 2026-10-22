#include "../../include/VideoPlayerScaffold/utils/RedisUtil.h"
#include <iostream>

int main(){
    RedisUtil::RedisConfig config{
        .host = "192.168.204.128",
        .password = "123456",
    };
    RedisUtil::RedisClient client(config);

    // ----------------------------------------------------
        // 测试 1：基础 SET 与 GET
        // ----------------------------------------------------
        std::cout << "\n[Test 1] 基础 SET 与 GET\n";
        std::string test_key = "test:user:1001";
        std::string test_val = "Alice";

        bool set_ok = client.set(test_key, test_val);
        std::cout << "SET " << test_key << " -> " << (set_ok ? "成功" : "失败") << "\n";

        auto get_val = client.get(test_key);
        if (get_val.has_value()) {
            std::cout << "GET " << test_key << " -> " << *get_val << "\n";
        } else {
            std::cerr << "GET " << test_key << " -> 未获取到数据\n";
        }

        // ----------------------------------------------------
        // 测试 2：EXISTS 检查
        // ----------------------------------------------------
        std::cout << "\n[Test 2] EXISTS 检查\n";
        bool exists_before = client.exists(test_key);
        std::cout << "Key 是否存在: " << (exists_before ? "是" : "否") << "\n";

        // ----------------------------------------------------
        // 测试 3：带 TTL（过期时间）验证
        // ----------------------------------------------------
        std::cout << "\n[Test 3] TTL 过期时间验证\n";
        std::string expire_key = "test:temp_token";
        // 写入一个 1000ms（1秒）过期的键
        client.set(expire_key, "token_123456", std::chrono::milliseconds(1000));

        std::cout << "刚写入时读取: " << (client.get(expire_key).value_or("已失效")) << "\n";
        std::cout << "等待 1.5 秒...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        auto expired_val = client.get(expire_key);
        std::cout << "过期后再次读取: " << (expired_val.has_value() ? *expired_val : "已成功过期 (nullopt)") << "\n";

        // ----------------------------------------------------
        // 测试 4：DEL 删除
        // ----------------------------------------------------
        std::cout << "\n[Test 4] DEL 删除键\n";
        bool del_ok = client.del(test_key);
        std::cout << "DEL " << test_key << " -> " << (del_ok ? "成功" : "失败") << "\n";

        bool exists_after = client.exists(test_key);
        std::cout << "删除后 Key 是否还存在: " << (exists_after ? "是" : "否") << "\n";

        // ----------------------------------------------------
        // 测试 5：raw() 逃生门调用原生接口
        // ----------------------------------------------------
        std::cout << "\n[Test 5] 使用 raw() 调用原生命令（Hash 与 List）\n";
        try {
            auto& raw_redis = client.raw();
            // 原生 Hash 操作
            raw_redis.hset("test:user_info", "age", "20");
            auto age = raw_redis.hget("test:user_info", "age");
            if (age) {
                std::cout << "原生 HGET age: " << *age << "\n";
            }

            // 清理 Hash 测试键
            raw_redis.del("test:user_info");
        } catch (const sw::redis::Error& err) {
            std::cerr << "原生调用异常: " << err.what() << "\n";
        }

        std::cout << "\n========== 测试全部执行完毕 ==========\n";
        return 0;
}
