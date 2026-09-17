#include <iostream>
#include <string>
#include "../../include/VideoPlayerScaffold/utils/AmqpUtil.h"

int main() {
    using namespace AmqpUtil;

    const std::string brokerUrl = "amqp://admin:123456@192.168.204.128:5672/";
    auto client = std::make_shared<MessageQueueClient>(brokerUrl);

    std::cout << "[Consumer] 正在连接 RabbitMQ..." << std::endl;
    if (!client->Start()) {
        std::cerr << "[Consumer] 连接 RabbitMQ 失败！" << std::endl;
        return -1;
    }
    std::cout << "[Consumer] 连接成功！" << std::endl;

    DeclareSettings settings{
        .exchangeName = "VideoTestExchange",
        .exchangeType = ExchangeType::Direct,
        .queueName = "VideoTestQueue",
        .bindingKey = "video.upload"
    };

    MessageSubscriber subscriber(client, settings);

    std::cout << "[Consumer] 正在订阅队列 " << settings.queueName << " ..." << std::endl;
    bool success = subscriber.Consume([](DeliveryContext& deliveryContext) {
        std::cout << "\n================ 收到新消息 ================" << std::endl;
        std::cout << "消息单号 (DeliveryTag): " << deliveryContext.GetDeliveryTag() << std::endl;
        std::cout << "是否为重发消息: " << (deliveryContext.IsRedelivered() ? "是" : "否") << std::endl;
        std::cout << "消息正文 (Payload): " << deliveryContext.GetMessageBody() << std::endl;

        // 模拟业务处理
        std::cout << "[业务处理] 正在执行视频转码任务..." << std::endl;

        // 业务成功后手动确认
        deliveryContext.Acknowleged();
        std::cout << "[Consumer] 消息已确认 (ACK)" << std::endl;
        std::cout << "============================================" << std::endl;
    }, 10, false);

    if (!success) {
        std::cerr << "[Consumer] 订阅失败！" << std::endl;
        return -1;
    }

    std::cout << "[Consumer] 订阅就绪，持续等待接收消息中 (按 Ctrl+C 退出)..." << std::endl;
    client->Wait();

    return 0;
}
