#include <amqpcpp/exchangetype.h>
#include <amqpcpp/flags.h>
#include <iostream>
#include <ev.h>
#include <string>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <amqpcpp/address.h>
#include <amqpcpp/linux_tcp/tcpchannel.h>
#include <amqpcpp/linux_tcp/tcpconnection.h>

int main(){
    const std::string url = "amqp://admin:123456@192.168.204.128:5672/";
    const std::string exchange = "video_event_exchange";
    const std::string queue = "video_transcode_queue";
    const std::string routing_key = "video.transcode";

    // 初始化底层循环事件
    struct ev_loop* loop = EV_DEFAULT;
    AMQP::LibEvHandler handler(loop);

    // 建立连接与信道
    AMQP::TcpConnection connection(&handler, AMQP::Address(url));
    AMQP::TcpChannel channel(&connection);

    // 全局错误处理函数
    channel.onError([loop](const char* message){
        std::cerr << "[AMQP] 通道发生异常: " << message << std::endl;
        ev_break(loop, EVBREAK_ALL);
    });

    // 声明交换机、队列、绑定
    channel.declareExchange(exchange, AMQP::direct);
    channel.declareQueue(queue);
    channel.bindQueue(exchange, queue, routing_key)
        .onSuccess([&channel, &connection, loop, exchange, routing_key, queue](){
            channel.publish(exchange, routing_key, "Hello RabbitMQ");
            std::cout << "[Producer]消息已发送, 等待队列确认!" << std::endl;

            channel.declareQueue(queue, AMQP::passive)
                .onSuccess([&channel, &connection, loop](const std::string& name, uint32_t msg_cnt, uint32_t){
                    std::cout << "[Producer]发送成功, 当前队列[" << name << "]消息数量: " << msg_cnt << std::endl;
                    channel.close();
                    connection.close();
                    ev_break(loop, EVBREAK_ALL);
            });
        });
    
    ev_loop(loop, 0);

    std::cout << "[Producer]测试完成, 程序退出!" << std::endl;

    return 0;
}