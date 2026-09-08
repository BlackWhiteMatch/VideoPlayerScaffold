# VideoPlayerScaffold

基于 C++17 构建的高性能音视频流媒体与分布式服务脚手架。

## 📌 项目概述

`VideoPlayerScaffold` 提供了开发高并发、分布式音视频处理服务所需的核心基础组件，涵盖基于 etcd 的服务注册发现与负载均衡、基于 bRPC 的 RPC 通信、基于 FFmpeg 的零编解码 HLS 视频切片以及高性能异步日志等能力。

## 🛠️ 核心模块

- **EtcdUtil (服务注册与发现)**:
  - `Provider`: 基于租约 (Lease) 实现节点注册、自动心跳保活 (KeepAlive) 与 RAII 优雅下线。
  - `Watcher`: 实时监听前缀节点变动，结合双容器结构与原子计数，提供线程安全且无锁高效的 $O(1)$ 轮询负载均衡 (Round-Robin)。
- **RpcUtil (微服务 RPC 通信)**:
  - 基于 Apache bRPC 与 Protobuf。
  - 提供 `Channel` 连接池与 `ServiceManager` 服务节点管理器。
  - `ClosureFactory` 自动管理异步回调生命周期，避免内存泄漏。
  - `RpcServerFactory` 快速构建 bRPC 服务端。
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
├── include/VideoPlayerScaffold/utils/   # 核心组件头文件 (EtcdUtil, RpcUtil, FFmpegUtil, Logger, JsonUtil)
├── src/utils/                           # 核心组件具体实现
├── example-utils/                       # 封装工具模块的集成测试 (EtcdUtil, FFmpegUtil, RpcUtil 等)
└── example/                             # 基础技术栈的原生 Demo (brpc, etcd, ffmpeg, spdlog, gtest 等)
```

## ⚙️ 环境依赖

- **C++ 标准**: C++17 及以上 (GCC/G++ 8.0+)
- **依赖库**:
  - `etcd-cpp-api` (服务注册与发现)
  - `brpc` & `protobuf` (RPC 通信)
  - `ffmpeg` (`libavformat`, `libavcodec`, `libavutil`)
  - `spdlog` & `fmt` (日志)
  - `jsoncpp` (JSON 处理)
  - `gflags` / `gtest`

## 🚀 快速使用

### 1. 服务注册与发现 (EtcdUtil)

```cpp
#include "VideoPlayerScaffold/utils/EtcdUtil.h"

// 服务端：注册服务并自动保活
EtcdUtil::Provider provider("http://127.0.0.1:2379", /*ttl=*/5);
provider.Register("user_service", "node-1", "127.0.0.1:9090");

// 客户端：监听服务并轮询获取节点
EtcdUtil::Watcher watcher("http://127.0.0.1:2379", "/user_service");
watcher.Start();
std::string node_addr = watcher.GetEndPoint(); // O(1) 轮询获取可用地址
```

### 2. HLS 视频切片与 M3U8 处理 (FFmpegUtil)

```cpp
#include "VideoPlayerScaffold/utils/FFmpegUtil.h"

// 视频无损快速切片
FFmpegUtil::HLSSegmenterConfig config;
config.target_segment_duration = 5;
FFmpegUtil::HLSSegmenter segmenter(config);
segmenter.Remux("input.mp4", "output.m3u8");

// M3U8 注入 CDN 前缀
FFmpegUtil::M3U8Parser parser;
parser.ParseString(m3u8_content);
parser.ApplyBaseURL("https://cdn.example.com/hls/");
std::string cdn_m3u8 = parser.Serialize();
```

### 3. 异步 RPC 调用 (RpcUtil)

```cpp
#include "VideoPlayerScaffold/utils/RpcUtil.h"

RpcUtil::ServiceManager manager;
manager.watch("calc_service");
manager.addNode("calc_service", "127.0.0.1:9000");

auto channel = manager.getNode("calc_service");
// 通过 RpcUtil::ClosureFactory::create 创建 lambda 异步回调...
```

## 🔨 编译运行示例

每个示例目录均附带独立的 `makefile`，编译产物统一输出至根目录 `bin/`：

```bash
# 编译并运行 EtcdUtil 测试示例
cd example-utils/EtcdUtil
make
../../bin/example-utils-EtcdUtil-demo/Service-demo  # 启动服务端
../../bin/example-utils-EtcdUtil-demo/Client-demo   # 启动客户端

# 编译并运行 FFmpegUtil 切片测试
cd example-utils/FFmpegUtil
make
../../bin/example-utils-FFmpegUtil-demo/main test.mp4 out.m3u8
```

## 📄 开源许可

[MIT License](LICENSE)
