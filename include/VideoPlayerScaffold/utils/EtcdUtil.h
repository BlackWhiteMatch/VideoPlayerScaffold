#pragma once
#include <atomic>
#include <cstdint>
#include <etcd/Response.hpp>
#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Watcher.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace EtcdUtil {
class Provider{
public:
    explicit Provider(const std::string etcd_url, int ttl = 3);
    ~Provider();
    
    // 静止移动拷贝
    Provider(const Provider&) = delete;
    Provider operator=(const Provider&) = delete;
    Provider(Provider&&) = default;
    Provider& operator=(Provider&&) noexcept = default;
    
    bool Register(
        const std::string& service_name,
        const std::string& node_name,
        const std::string& endpoint
    );

    void UnRegister();

    bool GetRegister() { return _register; }
    int GetLeaseID() { return _lease_id; }

private:
    bool DoRegister();

private:
    std::string _etcd_url;
    int _ttl;
    
    std::string _node_key;
    std::string _endpoint;

    std::unique_ptr<etcd::Client> _client;
    std::shared_ptr<etcd::KeepAlive> _keep_alive;

    int64_t _lease_id{0};
    bool _register = false;
};

class Watcher{
public:
    Watcher(const std::string& etcd_url, const std::string& service_prefix);
    ~Watcher();

    bool Start();
    void Stop();

    std::string GetEndPoint();
    std::vector<std::string> GetAllEndpoints();

private:
    bool FetchAllNodes();
    bool ReBuildEndPointVector();
    void OnServerChange(const etcd::Response& response);

private:
    std::string _etcd_url;
    std::string _service_prefix;

    std::unique_ptr<etcd::Client> _client;
    std::unique_ptr<etcd::Watcher> _watcher;

    mutable std::shared_mutex _mutex;
    std::unordered_map<std::string, std::string> _node_map;
    std::vector<std::string> _endpoint_vec;

    bool _isRunning{false};
    std::atomic_int _index{0};
};
}