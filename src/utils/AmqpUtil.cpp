#include <algorithm>
#include <amqpcpp/exchangetype.h>
#include <amqpcpp/message.h>
#include <amqpcpp/table.h>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <amqpcpp/address.h>
#include <amqpcpp/flags.h>
#include <amqpcpp/linux_tcp/tcpconnection.h>
#include <amqpcpp/linux_tcp/tcpchannel.h>
#include <memory>
#include <ev.h>
#include <future>
#include <mutex>
#include <queue>
#include "../../include/VideoPlayerScaffold/utils/AmqpUtil.h"

namespace AmqpUtil {
// -------------------------------------------------- DeliveryContext --------------------------------------------------
    void DeliveryContext::Acknowleged(bool acknowledgeMultiple) {
        if(!_isAcknowledgedOrRejected && _tcpChannel){
            int acknowledgeFlags = acknowledgeMultiple ? AMQP::multiple : 0;
            _tcpChannel->ack(_deliveryTag, acknowledgeFlags);
            _isAcknowledgedOrRejected = true;
        }
    }

    void DeliveryContext::Reject(bool shouldRequeue, bool rejectMutiple){
        if(_isAcknowledgedOrRejected && _tcpChannel) {
            int rejectFlags = 0;
            if(shouldRequeue) {
                rejectFlags |= AMQP::requeue;
            }
            if(rejectMutiple){
                rejectFlags |= AMQP::multiple;
            }
            _tcpChannel->reject(_deliveryTag, rejectFlags);
            _isAcknowledgedOrRejected = true;
        }
    }
// -------------------------------------------------- MessageQueueClient --------------------------------------------------
    MessageQueueClient::MessageQueueClient(const std::string& brokerAddressUrl)
        : _brokerAddressUrl(brokerAddressUrl)
    {
        _eventLoopHandle = ev_loop_new(EVFLAG_AUTO);
        ev_async_init(&_asyncEventWatcher, MessageQueueClient::OnAsyncNotificationTriggered);
        _asyncEventWatcher.data = this;
        ev_async_start(_eventLoopHandle, &_asyncEventWatcher);

        _libEventHandler = std::make_unique<AMQP::LibEvHandler>(_eventLoopHandle);
    }

    MessageQueueClient::~MessageQueueClient(){
        Stop();
        if(_eventLoopHandle){
            ev_loop_destroy(_eventLoopHandle);
            _eventLoopHandle = nullptr;
        }
    }

    bool MessageQueueClient::Start(){
        if(_isEventLoopRunning.exchange(true)){
            return true;
        }

        auto connectionEstablishedPromise = std::make_shared<std::promise<bool>>();
        auto connectionEstablishFuture = connectionEstablishedPromise->get_future();
        auto isPromiseFulfilled = std::make_shared<std::atomic<bool>>(false);

        _networkEventThread = std::thread([this, connectionEstablishedPromise, isPromiseFulfilled](){
            _tcpConnection = std::make_unique<AMQP::TcpConnection>(_libEventHandler.get(), AMQP::Address(_brokerAddressUrl));
            _tcpChannel = std::make_unique<AMQP::TcpChannel>(_tcpConnection.get());
            _tcpChannel->onError([this, connectionEstablishedPromise, isPromiseFulfilled](const char* errorMessagText){
                std::string readableErrorMessage = errorMessagText ? errorMessagText : "Unknown AMQP channel error";
                if(_errorNotificationCallback){
                    _errorNotificationCallback(readableErrorMessage);
                }
                if(!isPromiseFulfilled->exchange(true)){
                    connectionEstablishedPromise->set_value(false);
                }
            });

            _tcpChannel->onReady([this, connectionEstablishedPromise, isPromiseFulfilled]{
                _isConnetionEstablished = true;
                if(!isPromiseFulfilled->exchange(true)){
                    connectionEstablishedPromise->set_value(true);
                }
            });

            ev_run(_eventLoopHandle, 0);

            _tcpChannel.reset();
            _tcpConnection.reset();
        });

        if(connectionEstablishFuture.wait_for(std::chrono::seconds(5)) == std::future_status::ready){
            return connectionEstablishFuture.get();
        }

        return false;
    }

    void MessageQueueClient::Stop(){
        if(!_isEventLoopRunning.exchange(false)){
            return;
        }

        PostTask([this](){
            ev_async_stop(_eventLoopHandle, &_asyncEventWatcher);
            ev_break(_eventLoopHandle, EVBREAK_ALL);
        });

        if(_networkEventThread.joinable()){
            _networkEventThread.join();
        }
    }

    void MessageQueueClient::Wait(){
        if(_networkEventThread.joinable()){
            _networkEventThread.join();
        }
    }

    void MessageQueueClient::PostTask(std::function<void()> scheduledTask){
        if(!_isEventLoopRunning){
            return;
        }

        {
            std::lock_guard<std::mutex> lock(_taskQueueMutex);
            _pendingTaskQueue.push(std::move(scheduledTask));
        }
        ev_async_send(_eventLoopHandle, &_asyncEventWatcher);
    }

    void MessageQueueClient::OnAsyncNotificationTriggered(struct ev_loop* eventLoop, ev_async* asyncWatcher, int eventFlag){
        auto* selfInstance = static_cast<MessageQueueClient*> (asyncWatcher->data);
        std::queue<std::function<void()>>  localExecutableTasks;

        {
            std::lock_guard<std::mutex> taskLock(selfInstance->_taskQueueMutex);
            localExecutableTasks.swap(selfInstance->_pendingTaskQueue);
        }

        while(!localExecutableTasks.empty()){
            auto currentTask = std::move(localExecutableTasks.front());
            localExecutableTasks.pop();
            if(currentTask){
                currentTask();
            }
        }
    }

    AMQP::ExchangeType MessageQueueClient::ConvertToAmqpExchangeType(ExchangeType exchangeType){
        switch (exchangeType) {
            case ExchangeType::Direct : return AMQP::ExchangeType::direct;
            case ExchangeType::Topic : return AMQP::ExchangeType::topic;
            case ExchangeType::Fanout : return AMQP::ExchangeType::fanout;
            case ExchangeType::Headers : return AMQP::ExchangeType::headers;
            case ExchangeType::Delayed : return AMQP::ExchangeType::direct;
            default: return AMQP::ExchangeType::direct;
        }
    }

    bool MessageQueueClient::DeclareTopologyInternal(
        const std::string& exchangeName,
        AMQP::ExchangeType amqpExchangeType,
        const std::string& queueName,
        const std::string& bindingKey,
        const AMQP::Table& additionalArguments,
        std::chrono::milliseconds operationTimeout
    ){
        auto executionPromise = std::make_shared<std::promise<bool>>();
        auto executionFuture = executionPromise->get_future();

        PostTask([this, exchangeName, amqpExchangeType, queueName, bindingKey, additionalArguments, executionPromise](){
            if(!_tcpChannel || !_tcpChannel->connected()){
                executionPromise->set_value(false);
                return;
            }
            // 声明交换机
            _tcpChannel->declareExchange(exchangeName, amqpExchangeType, AMQP::durable)
                .onError([executionPromise](const char* errorMessageText){
                    executionPromise->set_value(false);
                })
                .onSuccess([this, exchangeName, queueName, bindingKey, additionalArguments, executionPromise](){
                    // 声明队列
                    _tcpChannel->declareQueue(queueName, AMQP::durable, additionalArguments)
                        .onError([executionPromise](const char* errorMessageText){
                            executionPromise->set_value(false);
                        })
                        .onSuccess([this, exchangeName, queueName, bindingKey, executionPromise](){
                            // 绑定队列到交换机
                            _tcpChannel->bindQueue(exchangeName, queueName, bindingKey)
                                .onError([executionPromise](const char* errorMessageText){
                                    executionPromise->set_value(false);
                                })
                                .onSuccess([executionPromise](){
                                    executionPromise->set_value(true);
                                });
                        });
                });
        });
        if(executionFuture.wait_for(operationTimeout) == std::future_status::ready){
            return executionFuture.get();
        }
        return false;
    }

    bool MessageQueueClient::Declare(
        const DeclareSettings& declareSettings,
        std::chrono::milliseconds operationTimeout
    ){
        // 创建延时死信队列
        if(declareSettings.exchangeType == ExchangeType::Delayed){
            // 创建通信队列
            AMQP::Table emptyArguments;
            if(!DeclareTopologyInternal(
                declareSettings.DeadLetterExchangeName(),
                AMQP::ExchangeType::direct,
                declareSettings.DeadLetterQueueName(),
                declareSettings.DeadLetterBindingKey(),
                emptyArguments,
                operationTimeout
                )
            ){
                return false;
            }
            // 创建死信队列
            AMQP::Table delayArgument;
            delayArgument["x-dead-letter-exchange"] = declareSettings.DeadLetterExchangeName();
            delayArgument["x-dead-letter-routing-key"] = declareSettings.DeadLetterBindingKey();
            delayArgument["x-message-ttl"] = static_cast<uint64_t>(declareSettings.delayedTimetoLiveMilliscond);

                return DeclareTopologyInternal(
                    declareSettings.DeadLetterExchangeName(),
                    AMQP::ExchangeType::direct,
                    declareSettings.DeadLetterQueueName(),
                    declareSettings.DeadLetterBindingKey(),
                    delayArgument,
                    operationTimeout
                );
        }

        // 创建正常队列
        AMQP::Table normalArgument;
        return DeclareTopologyInternal(
            declareSettings.exchangeName,
            ConvertToAmqpExchangeType(declareSettings.exchangeType),
            declareSettings.queueName,
            declareSettings.bindingKey,
            normalArgument,
            operationTimeout
        );
    }

    bool MessageQueueClient::Publish(const std::string &exchangeName, const std::string &rountKey, const std::string &messagePayload){
        if(!_isEventLoopRunning || !_isConnetionEstablished){
            return false;
        }

        PostTask([this, exchangeName, rountKey, messagePayload](){
            if(_tcpChannel && _tcpChannel->connected()){
                _tcpChannel->publish(exchangeName, rountKey, messagePayload);
            }
        });
        return true;
    }

    bool MessageQueueClient::Consume(
        const std::string& queueName,
        MessageConsumerCallback consumerCallback,
        uint16_t prefetchMessagCount,
        bool enableAutomicAckonwledge,
        std::chrono::milliseconds operationTimeout
    ){
        auto subscriptionPromise = std::make_shared<std::promise<bool>>();
        auto subscriptionFuture = subscriptionPromise->get_future();

        PostTask(
            [this, queueName, consumerCallback = std::move(consumerCallback),
            prefetchMessagCount, enableAutomicAckonwledge, subscriptionPromise](){
                if(!_tcpChannel || !_tcpChannel->connected()){
                    subscriptionPromise->set_value(false);
                    return;
                }

                _tcpChannel->setQos(prefetchMessagCount);

                int consumeFlag = enableAutomicAckonwledge ? AMQP::noack : 0;
                _tcpChannel->consume(queueName, consumeFlag)
                    .onMessage([this, consumerCallback, enableAutomicAckonwledge]
                    (
                        const AMQP::Message& message,
                        uint64_t deliveryTag,
                        bool isRedelivery
                    )
                    {
                        std::string receivedPayload(message.body(), message.bodySize());
                        DeliveryContext deliveryContext(_tcpChannel.get(), deliveryTag, std::move(receivedPayload), isRedelivery);

                        // 交付业务层
                        consumerCallback(deliveryContext);

                        if(!enableAutomicAckonwledge && !deliveryContext.IsAcknowledgedOrRejected()){
                            deliveryContext.Acknowleged();
                        }
                    })
                    .onError([subscriptionPromise](const char* errorMessageText){
                        subscriptionPromise->set_value(false);
                    })
                    .onSuccess([subscriptionPromise](){
                        subscriptionPromise->set_value(true);
                    });
            }
        );
        if(subscriptionFuture.wait_for(operationTimeout) == std::future_status::ready){
            return subscriptionFuture.get();
        }
        return false;
    }
    // -------------------------------------------------- MessageSubscriber --------------------------------------------------
    bool MessageSubscriber::Consume(MessageConsumerCallback consumerCallback, uint16_t prefetchMessageCount, bool enableAutomicAcknowlege){
        if(!_messageQueueClient->Declare(_declareSetting)){
            return false;
        }
        std::string targetQueueName = (_declareSetting.exchangeType == ExchangeType::Delayed)
            ? _declareSetting.DeadLetterQueueName() : _declareSetting.queueName;

        return _messageQueueClient->Consume(targetQueueName, std::move(consumerCallback), prefetchMessageCount, enableAutomicAcknowlege);
    }
}
