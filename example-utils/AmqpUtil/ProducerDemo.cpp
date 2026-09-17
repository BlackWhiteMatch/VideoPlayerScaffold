#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "../../include/VideoPlayerScaffold/utils/AmqpUtil.h"

int main() {
    using namespace AmqpUtil;

    const std::string brokerUrl = "amqp://admin:123456@192.168.204.128:5672/";
    auto client = std::make_shared<MessageQueueClient>(brokerUrl);

    std::cout << "[Producer] 正在连接 RabbitMQ..." << std::endl;
    if (!client->Start()) {
        std::cerr << "[Producer] 连接 RabbitMQ 失败！" << std::endl;
        return -1;
    }
    std::cout << "[Producer] 连接成功！" << std::endl;

    DeclareSettings settings{
        .exchangeName = "VideoTestExchange",
        .exchangeType = ExchangeType::Direct,
        .queueName = "VideoTestQueue",
        .bindingKey = "video.upload"
    };

    MessagePublisher publisher(client, settings);

    std::cout << "[Producer] 正在初始化拓扑结构 (声明交换机与队列)..." << std::endl;
    if (!publisher.Initialize()) {
        std::cerr << "[Producer] 拓扑初始化失败！" << std::endl;
        return -1;
    }
    std::cout << "[Producer] 拓扑就绪！" << std::endl;

    // 循环发送 5 条测试消息
    for (int i = 1; i <= 5; ++i) {
        std::string payload = "{\"videoId\": \"VID-2026-00" + std::to_string(i) + "\", \"format\": \"mp4\", \"quality\": \"1080p\"}";
        std::cout << "[Producer] 发送第 " << i << " 条消息: " << payload << std::endl;
        publisher.PublishMessage(payload);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "[Producer] 全部 5 条测试消息投递完成！" << std::endl;

    // 等待网络缓冲区冲洗
    std::this_thread::sleep_for(std::chrono::seconds(1));

    client->Stop();
    std::cout << "[Producer] 客户端已优雅关闭。" << std::endl;

    return 0;
}
