#include <amqpcpp/exchangetype.h>
#include <amqpcpp/flags.h>
#include <amqpcpp/message.h>
#include <cstdint>
#include <iostream>
#include <ev.h>
#include <string>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <amqpcpp/address.h>
#include <amqpcpp/linux_tcp/tcpchannel.h>
#include <amqpcpp/linux_tcp/tcpconnection.h>

static struct ev_loop* g_loop = nullptr;

void SigHandler(int) {
    std::cout << "\n>>> 接收到退出信号，正在退出..." << std::endl;
    if(ev_loop){
        ev_break(g_loop, EVBREAK_ALL);
    }
}

int main(){
    const std::string url = "amqp://admin:123456@192.168.204.128:5672/";
    const std::string exchange = "video_event_exchange";
    const std::string queue = "video_transcode_queue";
    const std::string routing_key = "video.transcode";

    // 初始化底层循环事件
    struct ev_loop* g_loop = EV_DEFAULT;
    AMQP::LibEvHandler handler(g_loop);

    // 建立连接与信道
    AMQP::TcpConnection connection(&handler, AMQP::Address(url));
    AMQP::TcpChannel channel(&connection);

    // 全局错误处理函数
    channel.onError([g_loop](const char* message){
        std::cerr << "[AMQP] 通道发生异常: " << message << std::endl;
        ev_break(g_loop, EVBREAK_ALL);
    });

    // 声明交换机、队列、绑定
    channel.declareExchange(exchange, AMQP::direct);
    channel.declareQueue(queue);
    channel.bindQueue(exchange, queue, routing_key);

    // 设置只查询一条信息
    channel.setQos(1);

    channel.consume(queue)
        .onReceived([&channel](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered){
            std::string payload(message.body(), message.bodySize());
            std::cout << "\n----------------------------------------" << std::endl;
            std::cout << "[收到消息]: " << payload << std::endl;
            std::cout << "[Tag编号]: " << deliveryTag << " | [是否重发]: " << (redelivered ? "是" : "否") << std::endl;
            std::cout << "----------------------------------------" << std::endl;

            channel.ack(deliveryTag);
        })
        .onSuccess([](const std::string& consumerTag){
            std::cout << ">>> 消费者注册成功，监听队列: video_transcode_queue" << std::endl;
            std::cout << ">>> 正在持续监听，按 Ctrl+C 可停止..." << std::endl;
        })
        .onError([](const char* message) {
            std::cerr << ">>> 注册消费监听失败: " << message << std::endl;
        });
        
    ev_run(g_loop, 0);
    
    channel.close();
    connection.close();
    
    std::cout << "消费者下线!" << std::endl; 

    return 0;
}