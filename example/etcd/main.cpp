#include <iostream>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <memory>
#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Watcher.hpp>
#include <etcd/Response.hpp>
#include <etcd/Value.hpp>

void OnServerChange(const etcd::Response& response) {
    for (const auto& ev : response.events()) {
        if (ev.event_type() == etcd::Event::EventType::PUT) {
            std::cout << "[online] " << ev.kv().key() << " -> " << ev.kv().as_string() << std::endl;
        } else if (ev.event_type() == etcd::Event::EventType::DELETE_) {
            std::cout << "[offline] " << ev.kv().key() << std::endl;
        }
    }
}

int main() {
    // 1. 确保带有 http:// 协议头
    std::string etcd_url = "http://192.168.204.128:2379";
    etcd::Client client(etcd_url);

    std::string service_prefix = "/services/user_service";
    std::string node_key = "/services/user_service/node-1";
    std::string node_endpoint = "192.168.204.2:9090";

    // 2. 启动 Watcher
    std::cout << ">>> 启动 Watcher 监听前缀: " << service_prefix << std::endl;
    etcd::Watcher watcher(client, service_prefix, OnServerChange, true);

    // 3. 显式申请 3 秒租约并严格检查返回值
    std::cout << ">>> 申请租约 (TTL = 3s)..." << std::endl;
    auto lease_resp = client.leasegrant(3).get();
    if (!lease_resp.is_ok()) {
        std::cerr << ">>> 申请租约失败! 错误信息: " << lease_resp.error_message() << std::endl;
        std::cerr << ">>> 请检查 etcd 服务端是否在 " << etcd_url << " 正常运行。" << std::endl;
        return -1;
    }

    int64_t lease_id = lease_resp.value().lease();
    std::cout << ">>> 成功拿到有效 Lease ID: " << lease_id << std::endl;

    // 4. 写入节点数据绑定租约
    auto put_resp = client.put(node_key, node_endpoint, lease_id).get();
    if (!put_resp.is_ok()) {
        std::cerr << ">>> 写入节点失败: " << put_resp.error_message() << std::endl;
        client.leaserevoke(lease_id).wait();
        return -1;
    }
    std::cout << ">>> 节点注册成功!" << std::endl;

    // 5. 显式构建 KeepAlive 心跳保活器
    auto error_handler = [](std::exception_ptr eptr) {
        try {
            if (eptr) std::rethrow_exception(eptr);
        } catch (const std::exception& e) {
            std::cerr << ">>> KeepAlive 出现异常: " << e.what() << std::endl;
        }
    };
    auto keepalive = std::make_shared<etcd::KeepAlive>(etcd_url, error_handler, 3, lease_id);

    // 6. 正常保活 6 秒
    std::cout << ">>> 服务正常运行中（模拟 6 秒，观察心跳保活）..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(6));

    // 7. 优雅注销
    std::cout << ">>> 模拟节点停止心跳 / 撤出租约..." << std::endl;
    keepalive->Cancel();
    if (lease_id > 0) {
        client.leaserevoke(lease_id).wait();
    }

    // 等待 2 秒捕获 DELETE 变更
    std::this_thread::sleep_for(std::chrono::seconds(2));
    watcher.Cancel();

    std::cout << ">>> Demo 顺利结束，无卡死！" << std::endl;
    return 0;
}