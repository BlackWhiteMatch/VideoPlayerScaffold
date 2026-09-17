# VideoPlayerScaffold

基于 C++17 构建的高性能音视频流媒体与分布式服务脚手架。

## 📌 项目概述

`VideoPlayerScaffold` 提供了开发高并发、分布式音视频处理服务所需的核心基础组件，涵盖基于 etcd 的服务注册发现与负载均衡、基于 bRPC 的 RPC 通信、基于 AMQP-CPP 的异步消息队列、基于 FFmpeg 的零编解码 HLS 视频切片以及高性能异步日志等能力。

## 🛠️ 核心模块

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

## 📂 目录结构

```text
VideoPlayerScaffold/
├── include/VideoPlayerScaffold/utils/   # 核心组件头文件 (AmqpUtil, EtcdUtil, RpcUtil, FFmpegUtil, Logger, JsonUtil)
├── src/utils/                           # 核心组件具体实现
├── example-utils/                       # 封装工具模块的集成测试 (AmqpUtil, EtcdUtil, FFmpegUtil, RpcUtil 等)
└── example/                             # 基础技术栈的原生 Demo (amqp, brpc, etcd, ffmpeg, spdlog, gtest 等)
