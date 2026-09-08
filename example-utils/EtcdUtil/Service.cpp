#include "../../include/VideoPlayerScaffold/utils/EtcdUtil.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

std::string etcd_url = "http://192.168.204.128:2379";
std::string service_name = "user_service";
std::string endpoint = "192.168.204.128:9090"; // 换成真实 IP:Port 方便 Watcher 测试
std::string node_name = "node-1";

std::atomic<bool> g_running{true};

void SigHandler(int) {
    g_running = false;
}

int main() {
    // 捕获中断信号，确保按下 Ctrl+C 后能跳出循环执行优雅注销
    std::signal(SIGINT, SigHandler);
    std::signal(SIGTERM, SigHandler);

    EtcdUtil::Provider provider(etcd_url);

    // 严谨校验注册状态
    if (!provider.Register(service_name, node_name, endpoint)) {
        std::cerr << ">>> 节点注册失败，退出！" << std::endl;
        return -1;
    }

    std::cout << ">>> 节点注册成功!" << std::endl
              << "    service_name: " << service_name << std::endl
              << "    node_name:    " << node_name << std::endl
              << "    endpoint:     " << endpoint << std::endl;
    std::cout << ">>> 节点已上线并维持心跳保活中，按 Ctrl+C 可停止节点..." << std::endl;

    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "\n>>> 正在执行下线注销..." << std::endl;
    provider.UnRegister();
    std::cout << ">>> 退出完成。" << std::endl;

    return 0;
}