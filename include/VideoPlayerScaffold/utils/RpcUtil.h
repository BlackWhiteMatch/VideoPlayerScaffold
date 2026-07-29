#pragma once
#include <brpc/callback.h>
#include <brpc/controller.h>
#include <functional>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <brpc/channel.h>

namespace RpcUtil {
    class Channel {
        public:
            explicit Channel(const std::string& addr)
                :_addr(addr), _raw_channel(std::make_shared<brpc::Channel>()){}
            Channel() = default;

            // 禁止拷贝，允许移动
            Channel(const Channel&) = delete;
            Channel operator=(const Channel&) = delete;
            
            bool Init(const brpc::ChannelOptions* options = nullptr);
            std::shared_ptr<brpc::Channel> get() const;
            brpc::Channel* raw() const;

            const std::string& address() const;
            bool is_initialized() const;
        private:
            std::string _addr;
            std::shared_ptr<brpc::Channel> _raw_channel;
            bool _is_initialized{false};
    };

    class Channels{
        public:
            Channels();
            Channels(const std::string& service_name);
            // 插入Channel
            void insert(const std::string& addr);
            // 移除Channel
            void remove(const std::string& addr);
            // 获取Channels
            std::shared_ptr<Channel> select();
            // 获取节点地址
            std::optional<std::string> selectAddr();
        private:
            std::mutex _mutex;
            std::string _service_name;
            uint32_t _index;
            std::vector<std::pair<std::string, std::shared_ptr<Channel>>> _channels;
    };

    class ServiceManager{
        public:
            ServiceManager() = default;
            ~ServiceManager() = default;

            ServiceManager(const ServiceManager&) = delete;
            ServiceManager& operator=(const ServiceManager&) = delete;

            void watch(const std::string& service_name);
            void addNode(const std::string& service_name, const std::string& addr);
            void removeNode(const std::string& service_name, const std::string& addr);
            
            std::shared_ptr<Channel> getNode(const std::string& service_name);
            std::optional<std::string> getNodeAddress(const std::string& service_name);

        private:
            std::shared_ptr<Channels> getChannels(const std::string& service_name);

            std::mutex _mutex;
            std::unordered_map<std::string, std::shared_ptr<Channels>> _map;
    };

    class ClosureFactory {
        public:
            static google::protobuf::Closure* create(std::function<void()> callback){
                std::shared_ptr<objectHepler> obj = std::make_shared<objectHepler>();
                obj->callback = std::move(callback);
                return brpc::NewCallback(&ClosureFactory::callbackHandleHelper, obj);
            }
        
        private:
            struct objectHepler{
                std::function<void()> callback;
            };
            
            static void callbackHandleHelper(const std::shared_ptr<objectHepler> obj){
                if(obj && obj->callback){
                    obj->callback();
                }
            }
    };

    class RpcServerFactory {
        public:
            static std::shared_ptr<brpc::Server> create(
                int port,
                google::protobuf::Service* service,
                int idle_timeout_sec = -1
            );
    };
}