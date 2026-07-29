#include "../../include/VideoPlayerScaffold/utils/RpcUtil.h"
#include "../../include/VideoPlayerScaffold/utils/Logger.h"
#include <brpc/channel.h>
#include <brpc/controller.h>
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <brpc/server.h>

namespace RpcUtil {
    // --------------------------------------- class Channel ---------------------------------------
    bool Channel::Init(const brpc::ChannelOptions* options) {
        brpc::ChannelOptions default_options;
        if(options == nullptr){
            default_options.protocol = "baidu_std";
            default_options.timeout_ms = 3000;
            default_options.max_retry = 3;
            options = &default_options;
        }
        
        if(_raw_channel->Init(_addr.c_str(), options) != 0){
            LOG_ERROR("Failed to initialize brpc channel for address: {}", _addr);
            return false;
        }

        _is_initialized = true;
        return true;
    }

    std::shared_ptr<brpc::Channel> Channel::get() const {
        return _raw_channel;
    }

    brpc::Channel* Channel::raw() const {
        return _raw_channel.get();
    }
    
    const std::string& Channel::address() const {
        return _addr;
    }

    bool Channel::is_initialized() const {
        return _is_initialized;
    }

    // --------------------------------------- class Channels ---------------------------------------
    Channels::Channels(): _index(0){}
    Channels::Channels(const std::string& service_name): _service_name(service_name), _index(0){}

    // 插入Channel
    void Channels::insert(const std::string& addr){
        std::unique_lock<std::mutex> lock(_mutex);
        // 查询，避免重复插入
        for(auto& item : _channels){
            if(item.first == addr){
                LOG_INFO("Node {} already exists in service: {}", addr, _service_name);
                return;
            }
        }

        // 如果addr不存在
        std::shared_ptr<Channel> newChannel = std::make_shared<Channel>(addr);
        if(!newChannel->Init()){
            LOG_ERROR("Failed to initialize channel for {} in service: {}", addr, _service_name);
            return;
        }

       _channels.push_back(std::make_pair(addr, newChannel));
       LOG_INFO("Successfully inserted node {} into service: {}", addr, _service_name);
    }

    // 移除Channel
    void Channels::remove(const std::string& addr){
        std::unique_lock<std::mutex> lock(_mutex);
        // 查找对应的node
        for(auto it = _channels.begin(); it != _channels.end(); ++it){
            if(it->first == addr){
                _channels.erase(it);
                LOG_INFO("Node: {} is removed from service: {}", addr, _service_name);
                return;
            }
        }
        LOG_WARN("Node {} not found in service: {}", addr, _service_name);
    }

    // 获取Channel
    std::shared_ptr<Channel> Channels::select(){
        std::unique_lock<std::mutex> lock(_mutex);
        if(_channels.empty()){
            LOG_ERROR("No available nodes in service: {}", _service_name);
            return nullptr;
        }

        size_t idx = (_index++) % _channels.size();
        
        return _channels[idx].second;
    }

    // 获取节点地址
    std::optional<std::string> Channels::selectAddr(){
        std::unique_lock<std::mutex> lock(_mutex);
        if(_channels.empty()){
            LOG_ERROR("No available nodes in service: {}", _service_name);
            return std::nullopt;
        }

        size_t idx = (_index++) % _channels.size();

        return _channels[idx].first;
    }

    // --------------------------------------- class ServiceManager ---------------------------------------
    std::shared_ptr<Channels> ServiceManager::getChannels(const std::string& service_name){
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _map.find(service_name);
        if(it != _map.end()){
            return it->second;
        }
        return nullptr;
    }

    void ServiceManager::watch(const std::string& service_name){
        std::unique_lock<std::mutex> lock(_mutex);
        std::shared_ptr<Channels> newChannels = std::make_shared<Channels>();
        _map.insert(std::make_pair(service_name, newChannels));
        LOG_INFO("Started watching service: {}", service_name);
    }

    void ServiceManager::addNode(const std::string& service_name, const std::string& addr){
        std::shared_ptr<Channels> curChannels = getChannels(service_name);
        if(curChannels == nullptr){
            LOG_WARN("Node online [ {} ], but service [ {} ] is not watched!", addr, service_name);
            return;
        }
        curChannels->insert(addr);
    }

    void ServiceManager::removeNode(const std::string& service_name, const std::string& addr){
        std::shared_ptr<Channels> curChannels = getChannels(service_name);
        if(curChannels == nullptr){
            LOG_WARN("Node offline [ {} ], but service [ {} ] is not watched!", addr, service_name);
            return;
        }
        curChannels->remove(addr);
    }
    
    std::shared_ptr<Channel> ServiceManager::getNode(const std::string& service_name){
        std::shared_ptr<Channels> curChannel = getChannels(service_name);
        if(curChannel == nullptr){
            LOG_ERROR("Service [ {} ] not found/watched!", service_name);
            return nullptr;
        }
        return curChannel->select();
    }

    std::optional<std::string> ServiceManager::getNodeAddress(const std::string& service_name){
        std::shared_ptr<Channels> curChannel = getChannels(service_name);
        if(curChannel == nullptr){
            LOG_ERROR("Service [ {} ] not found/watched!", service_name);
            return std::nullopt;
        }
        return curChannel->selectAddr();
    }

    // --------------------------------------- class RpcServerFactory ---------------------------------------
    std::shared_ptr<brpc::Server> RpcServerFactory::create(
        int port,
        google::protobuf::Service* service,
        int idle_timeout_sec
    ){
        if(service == nullptr){
            LOG_ERROR("Failed to create server: service pointer is nullptr!");
            return nullptr;
        }
        
        auto server = std::make_shared<brpc::Server>();
        
        if(server->AddService(service, brpc::SERVER_OWNS_SERVICE) != 0){
            LOG_ERROR("Failed to add service to brpc server!");
            return nullptr;
        }

        brpc::ServerOptions options;
        options.idle_timeout_sec = idle_timeout_sec;

        if(server->Start(port, &options) != 0){
            LOG_ERROR("Failed to start brpc server on port: ");
            return nullptr;
        }

        LOG_INFO("bRPC Server started successfully on port: {}", port);
        return server;
    }
}