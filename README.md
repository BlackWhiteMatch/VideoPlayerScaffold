# VideoPlayerScaffold

基于 C++17 构建的高性能音视频流媒体与分布式服务脚手架。

## 📌 项目概述

`VideoPlayerScaffold` 提供了开发高并发、分布式音视频处理服务所需的核心基础组件，涵盖：
- **基于 etcd 的服务注册发现与负载均衡** (`EtcdUtil`)
- **基于 bRPC 的高性能 RPC 通信与异步回调** (`RpcUtil`)
- **基于 AMQP-CPP 的异步事件驱动消息队列** (`AmqpUtil`)
- **基于 FastDFS 的分布式音视频文件与对象存储** (`FastDFSUtil`)
- **基于 redis-plus-plus 的高性能内存缓存与键值存储** (`RedisUtil`)
- **基于 JsonCpp 的 Elasticsearch 索引与查询 DSL 链式构建** (`ESUtil`) **[NEW]**
- **基于 ODB 的现代 C++ MySQL 对象关系映射与事务抽象** (`OdbUtil`) **[NEW]**
- **基于 FFmpeg 的零二次编解码 HLS 媒体流切片与 M3U8 解析** (`FFmpegUtil`)
- **基于 spdlog 的工业级高性能异步日志追踪** (`Logger`)

---

## 🛠️ 核心模块

- **ESUtil (Elasticsearch 索引映射与查询 DSL 链式构建工具) [NEW]**:
  - **解耦设计理念**: 将复杂的 ES RESTful JSON 报文生成与底层网络通信解耦，专注提供高性能、零多态切片风险、纯内存的语法构建器，配合已有客户端或原生 `elasticlient` 使用。
  - **`ESIndexBuilder` (索引建表映射构建器 / DDL)**:
    - 支持自定义分词器挂载（`AddCustomAnalyzer`，如将 `ikmax` 绑定至 `ik_max_word`）。
    - 支持严格动态映射控制（`SetDynamic(false / "strict")`），杜绝未定义字段引起的集群 Mapping 膨胀。
    - 细粒度字段属性配置（`AddField`）：指定字段类型（`text`, `keyword`, `integer`, `date` 等）、绑定分词器、设置相关度加权（`boost`）、以及通过 `is_index: false` 关闭特定字段（如头像 URL、生日）的倒排索引以节省存储与内存开销。
  - **`ESQueryBuilder` (检索 DSL 链式构建器 / DML)**:
    - **全量查询**: `MatchAll()` 极简全表扫描。
    - **Bool 复合检索**:
      - `must` 强匹配算分检索：`MustMatch`（单字段全文检索）、`MustMultiMatch`（跨多字段全文检索）、`MustTerm`（精确匹配打分）。
      - `filter` 零算分高效过滤（推荐主用）：`FilterTerm`（单值精确过滤）、`FilterTerms`（多值精确过滤，类似 SQL `IN`）、`FilterRange`（双边闭区间 `[gte, lte]`）、`FilterRangeGte`（单边下限 `>=`）、`FilterRangeLte`（单边上限 `<=`）。
      - `should` 加分或匹配：`ShouldMatch`、`ShouldTerm`、`ShouldTerms`、`ShouldRange` 等，结合 `SetMiniShouldMatch(count)` 精确控制最小命中门槛。
    - **分页与多级排序**: 支持 `SetFrom(offset)`、`SetSize(limit)` 以及多字段级联复合排序（`Sort(field, asc)`，按调用顺序严格保证数组优先级）。
  - **零胶水无缝集成 (`operator std::string()`)**:
    - 原生重载类型转换运算符，在将构建器直接传入 `elasticlient::Client::search` 等第三方网络接口（形参为 `const std::string&`）时，编译器在最后一刻自动触发 `Build()` 隐式转换为合法的 JSON 字符串，实现极致丝滑的调用体验。

- **OdbUtil (MySQL C++ ORM 数据库访问与事务管理) [NEW]**:
  - `MySQLSetting`: 结构化配置模型，支持指定主机、端口、数据库用户/密码、库名、字符集（`utf8mb4`）以及连接池大小（`pool_size`）。
  - **连接池管理 (`createDatabase`)**: 原生集成 `odb::mysql::connection_pool_factory` 连接池，提升数据库高并发连接复用，避免频繁建立 TCP 连接。
  - **现代事务包装器 (`executeTransaction` / `Client::execute`)**: 采用 C++17 `if constexpr` 与 `std::invoke_result_t` 自动推导并捕获 Lambda 返回值，结合 RAII 自动管理 `odb::transaction` 的开启与提交（commit），支持带返回值或 `void` 的任意复杂业务闭包，异常自动回滚。
  - **实体对象关系映射 (ORM)**: 支持 `student-odb` 等强类型对象模型映射，告别脆弱的裸 SQL 字符串拼接，提供编译期类型安全检查与视图查询（View）。

- **RedisUtil (高性能内存缓存与数据结构存储)**:
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

### 1. Elasticsearch 索引建表与 DSL 检索快速使用 [NEW]

```cpp
#include "VideoPlayerScaffold/utils/ESUtil.h"
#include <elasticlient/client.h>
#include <iostream>

int main() {
    // 1. 初始化第三方 elasticlient 客户端
    elasticlient::Client client({"http://127.0.0.1:9200"});

    // 2. 链式构建索引表结构 (DDL)
    std::string index_dsl = ESUtil::ESIndexBuilder()
        .AddCustomAnalyzer("ikmax", "ik_max_word")          // 挂载 IK 智能分词器
        .SetDynamic(false)                                  // 关闭动态映射，防止脏字段膨胀
        .AddField("user_id", "keyword")                     // 用户ID精确值
        .AddField("nickname", "text", "ikmax", 2.0)         // 昵称全文检索，权重加倍 2.0
        .AddField("phone", "keyword")                       // 手机号精确值
        .AddField("avatar_url", "keyword", "", 1.0, false)  // 头像只存储，不建立倒排索引
        .AddField("age", "integer")                         // 年龄数值
        .Build();

    client.index("users", "_doc", "default_id", index_dsl);

    // 3. 链式构建高级组合检索 (DML)
    std::vector<std::string> search_fields = {"nickname", "description"};
    std::vector<std::string> target_phones = {"15511111111", "15522222222"};

    // 隐式类型转换生效：直接将 ESQueryBuilder 实例当做 string 参数传给 client.search！
    auto resp = client.search("users", "_doc", 
        ESUtil::ESQueryBuilder()
            .MustMultiMatch(search_fields, "架构师") // 跨多字段全文检索
            .FilterTerms("phone", target_phones)     // 手机号精确多值过滤 (IN 语法)
            .FilterRange("age", 20, 35)              // 年龄区间: 20 <= age <= 35
            .Sort("age", true)                       // 第一优先级: 年龄升序
            .Sort("created_at", false)               // 第二优先级: 时间降序
            .SetFrom(0)                              // 分页偏移
            .SetSize(10)                             // 每页条数
    );

    std::cout << "ES 响应状态码: " << resp.status_code << std::endl;
    std::cout << "ES 检索结果: " << resp.text << std::endl;
    return 0;
}
```

### 2. ODB MySQL ORM 事务与数据操作快速使用 [NEW]

```cpp
#include "VideoPlayerScaffold/utils/OdbUtil.h"
#include <iostream>

// 引入 ODB 编译生成的实体头文件
// #include "student.hxx"
// #include "student-odb.hxx"

int main() {
    // 1. 初始化数据库连接配置与连接池
    OdbUtil::MySQL::MySQLSetting setting{
        .host = "127.0.0.1",
        .user = "root",
        .passwd = "123456",
        .db = "test_db",
        .port = 3306,
        .charset = "utf8mb4",
        .pool_size = 5
    };
    OdbUtil::MySQL::Client client(setting);

    // 2. 使用 lambda 闭包执行自动事务 (成功自动 commit，异常自动回滚)
    client.execute([&client]() {
        auto db = client.getDatabase();
        
        // 插入记录 (以 Student 为例)
        // Student stu(1001, "张三", 620.5);
        // db->persist(stu);
        
        std::cout << "事务自动提交完成！" << std::endl;
    });

    return 0;
}
```

### 3. Redis 缓存与多数据结构快速使用

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

### 4. FastDFS 分布式存储快速使用

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
│   ├── ESUtil.h                         # [NEW] Elasticsearch 索引与查询 DSL 链式构建器
│   ├── OdbUtil.h                        # [NEW] ODB MySQL ORM 连接池与事务自动包装器
│   ├── RedisUtil.h                      # Redis 封装 (连接池、超时控制、慢查询告警、raw 逃生门)
│   ├── FastDFSUtil.h                    # FastDFS 分布式文件存储封装
│   ├── AmqpUtil.h                       # AMQP 异步消息队列封装
│   ├── EtcdUtil.h                       # etcd 服务注册发现与负载均衡
│   ├── RpcUtil.h                        # bRPC 通信与异步回调封装
│   ├── FFmpegUtil.h                     # FFmpeg HLS 切片与 M3U8 解析
│   ├── Logger.h                         # spdlog 工业级高性能日志封装
│   └── JsonUtil.h                       # JsonCpp 封装
├── src/utils/                           # 核心组件具体实现
│   ├── ESUtil.cpp                       # [NEW] ESUtil 索引建表与查询构建实现
│   ├── RedisUtil.cpp                    # RedisUtil 接口实现与慢查询追踪
│   ├── FastDFSUtil.cpp                  # FastDFS 接口实现与内存自清理
│   ├── AmqpUtil.cpp                     # AMQP 事件循环与发布/订阅实现
│   ├── EtcdUtil.cpp                     # etcd 保活租约与 Watcher 轮询实现
│   ├── FFmpegUtil.cpp                   # FFmpeg 媒体流切片实现
│   ├── Logger.cpp                       # 日志单例与追踪实现
│   └── JsonUtil.cpp                     # JSON 序列化实现
├── example-utils/                       # 封装工具模块的集成测试
│   ├── ESUtil/                          # [NEW] ESUtil 综合单元测试 (建表/全量/组合过滤/Terms/Should/排序)
│   ├── RedisUtil/                       # RedisUtil 综合测试 (基础操作/TTL验证/raw逃生门)
│   ├── FastDFSUtil/                     # FastDFS 二次封装集成测试
│   ├── AmqpUtil/                        # AMQP 消息队列测试
│   ├── EtcdUtil/                        # 服务注册发现与负载均衡测试
│   ├── FFmpegUtil/                      # HLS 切片与 M3U8 解析测试
│   ├── RpcUtil/                         # bRPC 客户端/服务端测试
│   ├── JsonUtil/                        # JSON 工具测试
│   └── spdlog/                          # 日志工具测试
└── example/                             # 基础技术栈的原生 Demo 与学习案例
    ├── elasticsearch/                   # [NEW] elasticlient 原生 C++ 客户端增删改查演示
    ├── odb/                             # [NEW] ODB MySQL ORM 综合实战演练
    │   ├── CRUD/                        # ODB 增删改查实体操作演示
    │   ├── View/                        # ODB 视图映射与多表查询演示
    │   └── student_table/               # ODB 编译器生成的映射头文件与 SQL
    ├── redis/                           # Redis++ 原生多数据结构操作演示 (string/hash/list/set/zset)
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
| **C++ 标准** | `C++17` | 全局采用现代 C++ 规范 (`std::optional`, `if constexpr`, 类型推导等) |
| **全文检索** | `elasticlient`, `cpr`, `jsoncpp` | **[NEW]** Elasticsearch C++ 客户端驱动与请求体 JSON DSL 序列化 |
| **ORM / 数据库** | `libodb`, `libodb-mysql`, `libmysqlclient` | **[NEW]** 高性能编译期强类型 C++ ORM 框架与 MySQL 连接池 |
| **内存缓存/KV存储** | `redis++`, `hiredis` | Redis 官方推荐的现代 C++ 客户端驱动及底座 |
| **分布式存储** | `libfdfsclient`, `libfastcommon` | FastDFS 客户端及基础库 |
| **服务注册发现**| `etcd-cpp-api`, `protobuf`, `grpc` | etcd v3 客户端依赖 |
| **RPC 通信** | `brpc`, `protobuf`, `openssl` | 百度高性能 RPC 框架 |
| **消息队列** | `amqpcpp`, `libev` | 纯异步 AMQP 事件驱动 |
| **音视频处理** | `libavformat`, `libavcodec`, `libavutil` | FFmpeg 媒体流解封装与封装 |
| **日志与工具** | `spdlog`, `jsoncpp`, `pthread` | 日志追踪、JSON 处理与多线程 |

---

## 🔨 编译与运行集成用例

### 运行 ESUtil 检索构建器测试用例 [NEW]
```bash
cd example-utils/ESUtil/

# 编译并输出到 bin/ 隔离目录
make

# 运行测试套件（覆盖建表、全量检索、组合过滤、多值匹配、分页排序）
make run

# 清理编译产物
make clean
```

### 运行 Elasticsearch 原生客户端演练 [NEW]
```bash
cd example/elasticsearch/

# 编译并运行
make
make run
```

### 运行 ODB MySQL ORM 演练用例 [NEW]
```bash
# 1. 运行 CRUD 实体增删改查
cd example/odb/CRUD/
make
make run

# 2. 运行 View 视图查询
cd ../View/
make
make run
```

### 运行 RedisUtil 二次封装测试用例
```bash
cd example-utils/RedisUtil/

# 编译并输出到 bin/ 隔离目录
make
make run
make clean
```

### 运行 FastDFS 分布式存储测试用例
```bash
cd example-utils/FastDFSUtil/

make
make run
make clean
```
