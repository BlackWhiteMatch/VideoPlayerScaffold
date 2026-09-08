#include "../../include/VideoPlayerScaffold/utils/EtcdUtil.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

std::string etcd_url = "http://192.168.204.128:2379"; // 注意：etcd client 连接必须带上 http://
std::string service_prefix = "/user_service";

int main() {
    // 创建发现监听者
    EtcdUtil::Watcher watcher(etcd_url, service_prefix);
    if (!watcher.Start()) {
        std::cerr << ">>> 启动服务发现失败! Prefix: " << service_prefix << std::endl;
        return -1;
    }

    std::cout << ">>> 服务发现启动成功! 正在监听前缀: " << service_prefix << std::endl;
    std::cout << ">>> 开始持续监测节点列表，按 Ctrl+C 退出..." << std::endl;

    int count = 0;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        count++;

        std::string target = watcher.GetEndPoint();
        auto all_nodes = watcher.GetAllEndpoints();

        std::cout << "[Tick " << count << "] 在线节点数: " << all_nodes.size();
        if (target.empty()) {
            std::cout << " | 告警: 当前暂无可用节点!" << std::endl;
        } else {
            std::cout << " | 轮询命中 -> " << target << std::endl;
        }
    }

    watcher.Stop();
    return 0;
}