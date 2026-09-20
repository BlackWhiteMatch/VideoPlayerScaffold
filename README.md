# VideoPlayerScaffold

基于 C++17 构建的高性能音视频流媒体与分布式服务脚手架。

## 📌 项目概述

`VideoPlayerScaffold` 提供了开发高并发、分布式音视频处理服务所需的核心基础组件，涵盖基于 etcd 的服务注册发现与负载均衡、基于 bRPC 的 RPC 通信、基于 AMQP-CPP 的异步消息队列、基于 FastDFS 的分布式音视频文件存储、基于 FFmpeg 的零编解码 HLS 视频切片以及高性能异步日志等能力。

---

## 🛠️ 核心模块

- **FastDFSUtil (分布式文件/对象存储) [NEW]**:
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

### FastDFS 分布式存储快速使用

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
├── include/VideoPlayerScaffold/utils/   # 核心组件头文件 (FastDFSUtil, AmqpUtil, EtcdUtil, RpcUtil, FFmpegUtil, Logger, JsonUtil)
├── src/utils/                           # 核心组件具体实现
├── example-utils/                       # 封装工具模块的集成测试
│   ├── FastDFSUtil/                     # FastDFS 二次封装集成测试
│   ├── AmqpUtil/                        # AMQP 消息队列测试
│   ├── EtcdUtil/                        # 服务注册发现与负载均衡测试
│   ├── FFmpegUtil/                      # HLS 切片与 M3U8 解析测试
│   ├── RpcUtil/                         # bRPC 客户端/服务端测试
│   ├── JsonUtil/                        # JSON 工具测试
│   └── spdlog/                          # 日志工具测试
└── example/                             # 基础技术栈的原生 Demo
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
| **分布式存储** | `libfdfsclient`, `libfastcommon` | FastDFS 客户端及基础库 |
| **服务注册发现**| `etcd-cpp-api`, `protobuf`, `grpc` | etcd v3 客户端依赖 |
| **RPC 通信** | `brpc`, `protobuf`, `openssl` | 百度高性能 RPC 框架 |
| **消息队列** | `amqpcpp`, `libev` | 纯异步 AMQP 事件驱动 |
| **音视频处理** | `libavformat`, `libavcodec`, `libavutil` | FFmpeg 媒体流解封装与封装 |
| **日志与工具** | `spdlog`, `jsoncpp`, `pthread` | 日志追踪、JSON 处理与多线程 |

---

## 🔨 编译与运行集成用例

以运行新增的 **FastDFS 工具测试** 为例：

```bash
cd example-utils/FastDFSUtil/

# 编译并输出到 bin/ 隔离目录
make

# 运行测试
make run

# 清理编译产物
make clean
```
