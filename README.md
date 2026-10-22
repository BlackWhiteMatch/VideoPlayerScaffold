# VideoPlayerScaffold

基于 C++17 构建的高性能音视频流媒体与分布式服务脚手架。

## 📌 项目概述

`VideoPlayerScaffold` 提供了开发高并发、分布式音视频处理服务所需的核心基础组件，涵盖基于 etcd 的服务注册发现与负载均衡、基于 bRPC 的 RPC 通信、基于 AMQP-CPP 的异步消息队列、基于 FastDFS 的分布式音视频文件存储、**基于 redis-plus-plus 的高性能内存缓存与键值存储**、基于 FFmpeg 的零编解码 HLS 视频切片以及高性能异步日志等能力。

---

## 🛠️ 核心模块

- **RedisUtil (高性能内存缓存与数据结构存储) [NEW]**:
  - `RedisConfig`: 结构化配置模型，支持指定主机、端口、ACL 身份认证（`user` / `password`）、数据库号（`db`），内置毫秒级连接超时（`connection_timeout`）、读写超时（`socket_timeout`）、连接池容量（`pool_size`）与空闲回收时长（`pool_idle_interval`）。
  - **连接池管理**: 原生集成 `sw::redis::ConnectionPool` 连接池，提升高并发网络复用能力，并支持移动语义管理与 RAII 自动资源释放。
  - **慢查询监控与诊断 (`track_latency`)**: 内置非侵入式耗时追踪机制，自动捕获并打印执行耗时超过阈值（默认 `20.0ms`）的操作告警日志，助力分布式系统快速定位热点或慢命令。
  - **异常安全与空值安全**:
    - 全面捕获底层 `TimeoutError` 与 `Error` 异常，避免因网络抖动引起进程崩溃。
    - `get(key)` 采用现代 C++ `std::optional<std::string>` 返回，安全表达缓存命中与未命中。
  - **常用高频操作**: 提供简单直观的 `get`、`set`（支持毫秒级 TTL 自动过期）、`del`、`exists` 常用键值操作。
  - **逃生门机制 (`raw()`)**: 提供 `raw()` 接口直接暴露底层 `sw::redis::Redis&` 原生句柄，业务层可在享受统一配置与连接池管理的同时，无缝调用 Hash、List、Set、ZSet（有序集合）、Pipeline、事务及 Lua 脚本等全部高级命令。
- **FastDFSUtil (分布式文件/对象存储)**:
  - `FastDFSSetting`: 结构化配置模型，支持多 Tracker 集群节点配置、超时控制与连接池参数，告别对磁盘物理 `client.conf` 配置文件的强依赖。
  - `FastDFSClient`: 线程安全的静态工具类，内部使用 `std::mutex` 保证初始化与销毁的安全性。
  - **文件级操作**: `UploadFile`（本地文件直传，采用 `std::optional<std::string>` 返回 FileID）、`DownloadFile`（按 FileID 落地下载）、`DeleteFile`（服务端清理）。
  - **内存级操作（零磁盘 IO）**: `UploadBuffer`（支持内存字节数组或字符串直接上传）、`DownloadBuffer`（直接将远程文件读取至 `std::string`，底层自动接管 C 动态内存 `free`，防止内存泄漏）。
- **EtcdUtil (服务注册与发现)**:
  - `Provider`: 基于租约 (Lease) 实现节点注册、自动心跳保活 (KeepAlive) 与 RAII 优雅下线。
  - `Watcher`: 实时监听前缀节点变动，结合双容器结构与原子计数，提供线程安全且无锁高效的 $O(1)$ 轮询负载均衡 (Round-Robin)。
- **RpcUtil (微服务 RPC 通信)**:
  - 基于 Apache bRPC 与 Protobuf。
  - 提供 `Channel` 连接池与 `ServiceManager` 服务节点管理器。
  - `ClosureFactory` 自动管理异步回调生命周期，避免内存泄漏。
  - `RpcServerFactory` 快速构建 bRPC 服务端。
- **AmqpUtil (异步消息队列)**:
  - 基于 `AMQP-CPP` 与 `libev` 纯异步事件循环驱动。
  - `AmqpClient`: 线程安全的高性能 AMQP 客户端，封装底层复杂网络事件循环与多线程生命周期管理。
  - `MessagePublisher`: 支持 Direct/Topic/Fanout 等多种交换机类型的消息投递，提供可靠的异步 Publish 回调。
  - `MessageSubscriber`: 封装队列声明、交换机绑定与异步消费者监听，支持 DeliveryContext 上下文安全 ACK/NACK。
- **FFmpegUtil (音视频与流媒体处理)**:
  - `HLSSegmenter`: 零二次编码开销的转封装切片工具 (Remuxing)，自动校正 PTS/DTS 时间戳，输出标准 HLS (`.m3u8` / `.ts`)，支持进度回调。
  - `M3U8Parser`: 解析并修改 M3U8 文件，支持一键批量注入 CDN / 对象存储 URL 前缀。
- **Logger (工业级日志)**:
  - 基于 `spdlog` 封装，支持同步/异步日志队列、控制台彩色输出、文件输出及精确到文件名/行号/函数名的调用追踪。
- **JsonUtil (JSON 工具)**:
  - 基于 JsonCpp，提供采用 `std::optional` 的现代 C++ 序列化与反序列化接口。

---

## 💡 快速上手示例

### 1. Redis 缓存与多数据结构快速使用 [NEW]

```cpp
#include "VideoPlayerScaffold/utils/RedisUtil.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // 1. 初始化客户端配置（支持主机、端口、密码、超时及连接池参数）
    RedisUtil::RedisConfig config{
        .host = "127.0.0.1",
        .password = "123456",
        .slow_query_ms = 20.0, // 超过 20ms 触发慢查询报警
        .pool_size = 5
    };
    RedisUtil::RedisClient client(config);

    // 2. 基础 Key-Value 写入与读取（std::optional 防空指针）
    std::string key = "video:play_count:1001";
    client.set(key, "1024");
    if (auto val = client.get(key); val.has_value()) {
        std::cout << "播放量: " << *val << std::endl;
    }

    // 3. 带 TTL（毫秒级）自动过期
    std::string token_key = "user:session:token_abc";
    client.set(token_key, "uid_8888", std::chrono::milliseconds(2000)); // 2 秒过期

    // 4. 键存在性判断与删除
    if (client.exists(key)) {
        client.del(key);
        std::cout << "键已清理" << std::endl;
    }

    // 5. 使用 raw() 逃生门直接调用 Redis++ 原生高级数据结构（如 Hash、ZSet）
    try {
        auto& redis = client.raw();
        // 原生 Hash 操作
        redis.hset("video:info:1001", "title", "C++分布式流媒体实战");
        redis.hset("video:info:1001", "duration", "3600");

        // 原生 ZSet 热度排行榜
        redis.zadd("rank:video_hot", "video_1001", 99.5);
        redis.zadd("rank:video_hot", "video_1002", 95.0);
    } catch (const sw::redis::Error& e) {
        std::cerr << "Redis 操作异常: " << e.what() << std::endl;
    }

    return 0;
}
```

### 2. FastDFS 分布式存储快速使用

```cpp
#include "VideoPlayerScaffold/utils/FastDFSUtil.h"
#include <iostream>

int main() {
    // 1. 初始化客户端配置（支持多 Tracker 集群节点）
    FastDFSUtil::FastDFSSetting setting{
        .trackerServers = {"192.168.204.128:22122"}
    };
    if (!FastDFSUtil::FastDFSClient::Initialize(setting)) {
        std::cerr << "FastDFSClient 初始化失败！" << std::endl;
        return -1;
    }

    // 2. 上传文件（返回 std::optional<std::string> 形式的 FileID）
    auto uploadResult = FastDFSUtil::FastDFSClient::UploadFile("./sample.mp4");
    if (!uploadResult.has_value()) {
        std::cerr << "文件上传失败！" << std::endl;
        return -1;
    }
    std::string fileId = uploadResult.value();
    std::cout << "上传成功，File ID: " << fileId << std::endl;

    // 3. 下载文件（支持落盘到文件，或下载至内存 std::string 缓冲区）
    FastDFSUtil::FastDFSClient::DownloadFile(fileId, "./downloaded_sample.mp4");

    // 4. 删除文件
    if (FastDFSUtil::FastDFSClient::DeleteFile(fileId)) {
        std::cout << "文件清理成功！" << std::endl;
    }

    // 5. 退出时清理全局资源
    FastDFSUtil::FastDFSClient::Destory();
    return 0;
}
```

---

## 📂 目录结构

```text
VideoPlayerScaffold/
├── include/VideoPlayerScaffold/utils/   # 核心组件头文件
│   ├── RedisUtil.h                      # [NEW] Redis 封装 (连接池、超时控制、慢查询告警、raw 逃生门)
│   ├── FastDFSUtil.h                    # FastDFS 分布式文件存储封装
│   ├── AmqpUtil.h                       # AMQP 异步消息队列封装
│   ├── EtcdUtil.h                       # etcd 服务注册发现与负载均衡
│   ├── RpcUtil.h                        # bRPC 通信与异步回调封装
│   ├── FFmpegUtil.h                     # FFmpeg HLS 切片与 M3U8 解析
│   ├── Logger.h                         # spdlog 工业级高性能日志封装
│   └── JsonUtil.h                       # JsonCpp 封装
├── src/utils/                           # 核心组件具体实现
│   ├── RedisUtil.cpp                    # [NEW] RedisUtil 接口实现与慢查询追踪
│   ├── FastDFSUtil.cpp                  # FastDFS 接口实现与内存自清理
│   ├── AmqpUtil.cpp                     # AMQP 事件循环与发布/订阅实现
│   ├── EtcdUtil.cpp                     # etcd 保活租约与 Watcher 轮询实现
│   ├── FFmpegUtil.cpp                   # FFmpeg 媒体流切片实现
│   ├── Logger.cpp                       # 日志单例与追踪实现
│   └── JsonUtil.cpp                     # JSON 序列化实现
├── example-utils/                       # 封装工具模块的集成测试
│   ├── RedisUtil/                       # [NEW] RedisUtil 综合测试 (基础操作/TTL验证/raw逃生门)
│   ├── FastDFSUtil/                     # FastDFS 二次封装集成测试
│   ├── AmqpUtil/                        # AMQP 消息队列测试
│   ├── EtcdUtil/                        # 服务注册发现与负载均衡测试
│   ├── FFmpegUtil/                      # HLS 切片与 M3U8 解析测试
│   ├── RpcUtil/                         # bRPC 客户端/服务端测试
│   ├── JsonUtil/                        # JSON 工具测试
│   └── spdlog/                          # 日志工具测试
└── example/                             # 基础技术栈的原生 Demo 与学习案例
    ├── redis/                           # [NEW] Redis++ 原生多数据结构操作演示
    │   ├── string/                      # 字符串与自增自减
    │   ├── hash/                        # 哈希表操作
    │   ├── list/                        # 列表与队列操作
    │   ├── set/                         # 集合操作
    │   └── zset/                        # 有序集合与排行榜
    ├── fastdfs/                         # FastDFS 原生 C API 调用 Demo
    ├── amqp/                            # AMQP-CPP 原生 Demo
    ├── brpc/                            # bRPC 原生 Demo
    ├── etcd/                            # etcd-cpp-api 原生 Demo
    ├── ffmpeg/                          # FFmpeg 原生切片 Demo
    ├── gflags/                          # 命令行参数解析 Demo
    ├── gtest/                           # 单元测试框架 Demo
    └── ...
```

---

## 📦 依赖环境

| 组件 | 对应库 | 说明 |
| :--- | :--- | :--- |
| **C++ 标准** | `C++17` | 全局采用现代 C++ 规范 (`std::optional`, 初始化列表等) |
| **内存缓存/KV存储** | `redis++`, `hiredis` | Redis 官方推荐的现代 C++ 客户端驱动及底座 |
| **分布式存储** | `libfdfsclient`, `libfastcommon` | FastDFS 客户端及基础库 |
| **服务注册发现**| `etcd-cpp-api`, `protobuf`, `grpc` | etcd v3 客户端依赖 |
| **RPC 通信** | `brpc`, `protobuf`, `openssl` | 百度高性能 RPC 框架 |
| **消息队列** | `amqpcpp`, `libev` | 纯异步 AMQP 事件驱动 |
| **音视频处理** | `libavformat`, `libavcodec`, `libavutil` | FFmpeg 媒体流解封装与封装 |
| **日志与工具** | `spdlog`, `jsoncpp`, `pthread` | 日志追踪、JSON 处理与多线程 |

---

## 🔨 编译与运行集成用例

### 运行 RedisUtil 二次封装测试用例
```bash
cd example-utils/RedisUtil/

# 编译并输出到 bin/ 隔离目录
make

# 运行测试
make run

# 清理编译产物
make clean
```

### 运行 Redis 原生数据结构演练（以 ZSet 有序集合为例）
```bash
cd example/redis/zset/

# 编译并运行
make
make run
```

### 运行 FastDFS 分布式存储测试用例
```bash
cd example-utils/FastDFSUtil/

make
make run
make clean
```
