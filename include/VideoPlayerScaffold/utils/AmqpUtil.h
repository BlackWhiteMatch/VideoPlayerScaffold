#pragma once
#include <amqpcpp.h>
#include <amqpcpp/table.h>
#include <ev.h>
#include <amqpcpp/libev.h>
#include <algorithm>
#include <amqpcpp/callbacks.h>
#include <amqpcpp/connection.h>
#include <amqpcpp/exchangetype.h>
#include <amqpcpp/linux_tcp/tcpchannel.h>
#include <amqpcpp/linux_tcp/tcphandler.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <chrono>
#include <functional>
#include <atomic>
#include <pthread.h>
#include <queue>
#include <mutex>
#include <thread>

namespace AmqpUtil {

    // 交换机类型
    enum ExchangeType{
        Direct,
        Topic,
        Fanout,
        Headers,
        Delayed
    };

    // 配置结构体
    struct DeclareSettings {
        std::string exchangeName;
        ExchangeType exchangeType = ExchangeType::Direct;
        std::string queueName;
        std::string bindingKey;
        uint32_t delayedTimetoLiveMilliscond = 0;

        std::string DeadLetterExchangeName() const { return "dead_letter_" + exchangeName; }
        std::string DeadLetterQueueName() const { return "dead_letter_" + queueName; }
        std::string DeadLetterBindingKey() const { return "dead_letter_" + bindingKey; }
    };
// -------------------------------------------------- DeliveryContext --------------------------------------------------
    class DeliveryContext {
    public:
        DeliveryContext(
            AMQP::TcpChannel* tcpChannel,
            uint64_t deliveryTag,
            std::string messageBody,
            bool isRedelivered
        )   : _tcpChannel(tcpChannel)
            , _deliveryTag(deliveryTag)
            , _messageBody(messageBody)
            , _isRedelivered(isRedelivered)
            , _isAcknowledgedOrRejected(false){}

        // Getter
        const std::string& GetMessageBody() { return _messageBody; }
        uint64_t GetDeliveryTag() const { return _deliveryTag; }
        bool IsRedelivered() { return _isRedelivered; }
        bool IsAcknowledgedOrRejected() { return _isAcknowledgedOrRejected; }

        // 处理消息
        void Acknowleged(bool acknowledgeMultiple = false);
        void Reject(bool shouldRequeue = false, bool rejectMutiple = false);

    private:
        AMQP::TcpChannel* _tcpChannel;
        uint64_t _deliveryTag;
        std::string _messageBody;
        bool _isRedelivered;
        bool _isAcknowledgedOrRejected;
    };

    using MessageConsumerCallback = std::function<void(DeliveryContext& deliveryContext)>;
    using ErrorNotificationCallback = std::function<void(const std::string& errorMessage)>;
// -------------------------------------------------- MessageQueueClient --------------------------------------------------

    class MessageQueueClient : public std::enable_shared_from_this<MessageQueueClient> {
    public:
        using sharedPointer = std::shared_ptr<MessageQueueClient>;

        explicit MessageQueueClient(const std::string& brokerAddressUrl);
        ~MessageQueueClient();

        // 禁止移动拷贝
        MessageQueueClient(const MessageQueueClient&) = delete;
        MessageQueueClient& operator=(const MessageQueueClient&) = delete;

        bool Start();
        void Stop();
        void Wait();

        bool Declare(
            const DeclareSettings& declareSettings,
            std::chrono::milliseconds operationTimeout = std::chrono::seconds(5)
        );
        bool Publish(
            const std::string& exchangeName,
            const std::string& rountKey,
            const std::string& messagePayload
        );
        bool Consume(
            const std::string& queueName,
            MessageConsumerCallback consumerCallback,
            uint16_t prefetchMessagCount = 50,
            bool enableAutomicAckonwledge = false,
            std::chrono::milliseconds operationTimeout = std::chrono::seconds(5)
        );

        void SetErrorNotificationCallback(ErrorNotificationCallback errorNotificationCallback){
            _errorNotificationCallback = std::move(errorNotificationCallback);
        }

        void PostTask(std::function<void()> scheduleTask);
    private:
        static void OnAsyncNotificationTriggered(struct ev_loop* eventLoop, ev_async* asyncWatcher, int eventFlag);
        static AMQP::ExchangeType ConvertToAmqpExchangeType(ExchangeType exchangeType);

        bool DeclareTopologyInternal(
            const std::string& exchangeName,
            AMQP::ExchangeType amqpExchangeType,
            const std::string& queueName,
            const std::string& bindingKey,
            const AMQP::Table& additionalArguments,
            std::chrono::milliseconds operationTimeout
        );
    private:
        std::string _brokerAddressUrl;
        std::atomic_bool _isEventLoopRunning{false};
        std::atomic_bool _isConnetionEstablished{false};

        struct ev_loop* _eventLoopHandle{nullptr};
        ev_async _asyncEventWatcher;
        std::unique_ptr<AMQP::LibEvHandler> _libEventHandler;
        std::unique_ptr<AMQP::TcpConnection> _tcpConnection;
        std::unique_ptr<AMQP::TcpChannel> _tcpChannel;

        std::thread _networkEventThread;

        std::mutex _taskQueueMutex;
        std::queue<std::function<void()>> _pendingTaskQueue;

        ErrorNotificationCallback _errorNotificationCallback;
    };
// -------------------------------------------------- Publisher --------------------------------------------------
class MessagePublisher {
public:
    using SharedPointer = std::shared_ptr<MessagePublisher>;

    MessagePublisher(MessageQueueClient::sharedPointer messageQueueClient, DeclareSettings declareSetting)
        : _messageQueueClient(std::move(messageQueueClient))
        , _declareSetting(std::move(declareSetting)){}

    bool Initialize() {
        return _messageQueueClient->Declare(_declareSetting);
    }

    bool PublishMessage(const std::string& messagePoyload){
        return _messageQueueClient->Publish(_declareSetting.exchangeName, _declareSetting.bindingKey.empty() ? _declareSetting.queueName : _declareSetting.bindingKey, messagePoyload);
    }

private:
    MessageQueueClient::sharedPointer _messageQueueClient;
    DeclareSettings _declareSetting;
};
// -------------------------------------------------- MessageSubscriber --------------------------------------------------
class MessageSubscriber {
public:
    using SharedPointer = std::shared_ptr<MessageSubscriber>;

    MessageSubscriber(MessageQueueClient::sharedPointer messageQueueClient, DeclareSettings declareSetting)
        : _messageQueueClient(std::move(messageQueueClient))
        , _declareSetting(std::move(declareSetting)){}

    bool Consume(MessageConsumerCallback consumerCallback, uint16_t prefetchMessageCount = 50, bool enableAutomicAcknowlege = false);

private:
    MessageQueueClient::sharedPointer _messageQueueClient;
    DeclareSettings _declareSetting;
};
}
