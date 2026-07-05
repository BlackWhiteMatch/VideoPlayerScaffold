#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <iostream>

namespace utils {
    struct LoggerConfig {
        bool use_async = false;                                                         // 是否启用异步日志
        bool enable_console = true;                                                     // 是否输出到控制台
        bool enable_file = false;                                                       // 是否输出到文件
        std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%# %!] %v";          // 日志格式
        spdlog::level::level_enum level = spdlog::level::trace;                         // 默认日志级别
        spdlog::level::level_enum flush_level = spdlog::level::err;                     // 刷新磁盘的级别
        std::string file_path = "logs/app.log";                                         // 日志文件路径
        size_t async_queue_size = 8192;                                                 // 异步队列大小 (最好是 2 的幂次)
        size_t async_thread_count = 1;                                                  // 异步后台线程数
    };

    class Logger {
    public:
        ~Logger() = default;
        Logger() = default;

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        
        void Init(const LoggerConfig& config = LoggerConfig{});

        bool should_log(spdlog::level::level_enum level){
            if(auto logger = spdlog::default_logger()){
                return logger->should_log(level);
            }
            return false;
        }

        void set_pattern(const std::string& pattern) {
            if(spdlog::default_logger()){
                spdlog::default_logger()->set_pattern(pattern);
            }
        }

        void set_level(spdlog::level::level_enum level) {
            if(spdlog::default_logger()){
                spdlog::default_logger()->set_level(level);
            }
        }

        bool valid(const std::string& level_str){
            auto lvl = spdlog::level::from_str(level_str);
            auto real = spdlog::level::to_string_view(lvl);
            if(level_str == "warning" || level_str == "error"){
                return true;
            }
                
            return level_str == real;
        }
        void set_level(const std::string& level_str){
            auto level = spdlog::level::from_str(level_str);
            if(valid(level_str)){
                set_level(level);
            }else{
                set_level(spdlog::level::info);
                std::cerr << "[warn] 日志级别" << level_str << "无效，使用默认info" << std::endl;
            }
        }
        
        template<typename... Args>
        void log(spdlog::source_loc loc, spdlog::level::level_enum level, fmt::format_string<Args...> fmt, Args&&... args) {
            if(spdlog::default_logger())
                spdlog::default_logger()->log(loc, level, fmt, std::forward<Args>(args)...);
        }
    };

    extern Logger gLogger;
};

#define LOG_TRACE(...) do{\
    if(utils::gLogger.should_log(spdlog::level::trace))\
        utils::gLogger.log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::trace, ##__VA_ARGS__);\
}while(0)

#define LOG_INFO(...) do{\
    if(utils::gLogger.should_log(spdlog::level::info))\
        utils::gLogger.log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::info, ##__VA_ARGS__);\
}while(0)

#define LOG_ERROR(...) do{\
    if(utils::gLogger.should_log(spdlog::level::err))\
        utils::gLogger.log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::err, ##__VA_ARGS__);\
}while(0)

#define LOG_DEBUG(...) do{\
    if(utils::gLogger.should_log(spdlog::level::debug))\
        utils::gLogger.log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::debug, ##__VA_ARGS__);\
}while(0)

#define LOG_WARN(...) do{\
    if(utils::gLogger.should_log(spdlog::level::warn))\
        utils::gLogger.log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::warn, ##__VA_ARGS__);\
}while(0)

#define LOG_CRITICAL(...) do{\
    if(utils::gLogger.should_log(spdlog::level::critical))\
        utils::gLogger.log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::critical, ##__VA_ARGS__);\
}while(0)
