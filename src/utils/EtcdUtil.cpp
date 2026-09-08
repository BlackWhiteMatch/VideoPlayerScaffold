#include "../../include/VideoPlayerScaffold/utils/EtcdUtil.h"
#include "../../include/VideoPlayerScaffold/utils/Logger.h"
#include <atomic>
#include <etcd/Value.hpp>
#include <exception>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace EtcdUtil {
// -------------------------------------------- Provider --------------------------------------------

Provider::Provider(const std::string etcd_url, int ttl)
    :_etcd_url(etcd_url), _ttl(ttl)
{
    _client = std::make_unique<etcd::Client>(etcd_url);
}

Provider::~Provider(){
    UnRegister();
}

bool Provider::DoRegister(){
    // 申请租约
    auto lease_reponse = _client->leasegrant(_ttl).get();
    if(!lease_reponse.is_ok()){
        LOG_ERROR("[Provider] 申请租约失败: {}", lease_reponse.error_message());
        return false;
    }
    _lease_id = lease_reponse.value().lease();

    LOG_INFO("[Provider] 当前获取到的 Lease ID 为: {}", _lease_id);

    // 绑定写入
    auto put_response = _client->put(_node_key, _endpoint, _lease_id).get();
    if(!put_response.is_ok()){
        LOG_ERROR("[Provider]写入节点失败: {}", put_response.error_message());
        _client->leaserevoke(_lease_id).wait();
        _lease_id = 0;
        return false;
    }

    // 保活+异常处理
    auto HanderError = [this](std::exception_ptr exptr){
        try{
            if(exptr) std::rethrow_exception(exptr);
        } catch(const std::exception& e){
            LOG_WARN("[Provider]节点出现异常: {} , 正在尝试重连!", e.what());
            if(_register){
                this->DoRegister();
            }
        }
    };

    _keep_alive = std::make_shared<etcd::KeepAlive>(_etcd_url, HanderError ,_ttl, _lease_id);
    
    return true;
}

bool Provider::Register(
        const std::string& service_name,
        const std::string& node_name,
        const std::string& endpoint
){
    _node_key = "/" + service_name + "/" + node_name;
    _endpoint = endpoint;
    _register = true;

    return DoRegister();
}

void Provider::UnRegister(){
    if(!_register){
        return;
    }
    _register = false;
    if(_keep_alive){
        _keep_alive->Cancel();
        _keep_alive.reset();
    }
    if(_lease_id > 0 && _client){
        _client->leaserevoke(_lease_id).wait();
        _lease_id = 0;
    }
    LOG_INFO("[Provider]节点已主动下线！");
}

// -------------------------------------------- Watcher --------------------------------------------
Watcher::Watcher(const std::string& etcd_url, const std::string& service_prefix)
    :_etcd_url(etcd_url), _service_prefix(service_prefix)
{
    _client = std::make_unique<etcd::Client>(_etcd_url);
}

Watcher::~Watcher(){
    Stop();
}

bool Watcher::Start(){
    if(_isRunning){
        return true;
    }

    // 冷启动查找目录下的所有节点
    if(!FetchAllNodes()){
        LOG_WARN("[Watcher] 查找节点失败, 目录: {}", _service_prefix);
        return false;
    }

    try {
        _watcher = std::make_unique<etcd::Watcher>(
            *_client,
            _service_prefix,
            [this](const etcd::Response& response){ this->OnServerChange(response); },
            true
        );
        return true;
    }catch(const std::exception& e){
        LOG_ERROR("[Watcher] 创建监听通道异常: {}", e.what());
        return false;
    }

    return true;
}

bool Watcher::ReBuildEndPointVector(){
    _endpoint_vec.clear();
    _endpoint_vec.reserve(_node_map.size());
    for(const auto& [key, val] : _node_map)
    {
        _endpoint_vec.push_back(val);
    }
    return true;
}

bool Watcher::FetchAllNodes(){
    auto result_response = _client->ls(_service_prefix).get();
    if(!result_response.is_ok()){
        return false;
    }

    std::unique_lock<std::shared_mutex> lock(_mutex);
    _node_map.clear();
    for(size_t i = 0; i < result_response.keys().size(); i++){
        const std::string& key = result_response.key(i);
        const std::string& val = result_response.value(i).as_string();
        _node_map[key] = val;
        LOG_INFO("[Watcher]加载节点 [{}] -> [{}]", key, val);
    }

    return ReBuildEndPointVector();
}

void Watcher::OnServerChange(const etcd::Response& response){
    std::unique_lock<std::shared_mutex> lock(_mutex);
    bool change = false;
    
    for(const auto& ev : response.events()){
        const std::string& key = ev.kv().key();
        if(ev.event_type() == etcd::Event::EventType::PUT){
            _node_map[key] = ev.kv().as_string();
            change = true;
            LOG_INFO("[Watcher] 节点上线/更新 [{}] -> [{}]", key, ev.kv().as_string());
        }else if(ev.event_type() == etcd::Event::EventType::DELETE_){
            _node_map.erase(key);
            change = true;
            LOG_INFO("[Watcher] 节点{}下线", key);
        }
    }
    if(change){
        ReBuildEndPointVector();
    }
}

void Watcher::Stop(){
    if(!_isRunning){
        return;
    }
    _isRunning = false;
    
    if(_watcher){
        _watcher->Cancel();
        _watcher.reset();
    }

    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        _node_map.clear();
        _endpoint_vec.clear();
    }

    LOG_INFO("[Watcher] 监听已安全停止: {}", _service_prefix);
}

std::string Watcher::GetEndPoint(){
    std::shared_lock<std::shared_mutex> lock(_mutex);
    if(_endpoint_vec.empty()){
        LOG_WARN("[Watcher] 当前无可用节点，前缀: [{}]", _service_prefix);
        return "";
    }
    
    size_t idx = _index.fetch_add(1, std::memory_order_relaxed);
    return _endpoint_vec[idx % _endpoint_vec.size()];
}

std::vector<std::string> Watcher::GetAllEndpoints(){
    std::shared_lock<std::shared_mutex> lock(_mutex);
    return _endpoint_vec;
}

}