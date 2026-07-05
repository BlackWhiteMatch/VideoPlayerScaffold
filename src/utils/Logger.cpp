#include "../../include/VideoPlayerScaffold/utils/Logger.h"
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <vector>

namespace utils {

    Logger gLogger;

    void Logger::Init(const LoggerConfig& config) {
        try {
            std::vector<spdlog::sink_ptr> sinks;

            if(config.enable_console){
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                console_sink->set_level(spdlog::level::debug);
                sinks.push_back(console_sink);
            }

            if(config.enable_file){
                auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config.file_path, true);
                file_sink->set_level(spdlog::level::debug);
                sinks.push_back(file_sink);
            }
            // 防止什么都没选，导致日志输出不生效
            if(sinks.empty()){
                sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
            }

            std::shared_ptr<spdlog::logger> main_logger = nullptr;

            if(config.use_async){
                spdlog::init_thread_pool(config.async_queue_size, config.async_thread_count);
                main_logger = std::make_shared<spdlog::async_logger>("logbal_logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
            } else {
                main_logger = std::make_shared<spdlog::logger>("VideoPlayerScaffold", sinks.begin(), sinks.end());
            }

            // 统一设置日志格式: [时间] [级别] [文件名:行号 函数名] 内容
            main_logger->set_pattern(config.pattern);
            main_logger->set_level(config.level); // Logger 总闸允许放行的最低级别
            
            // 设为全局默认 Logger
            spdlog::set_default_logger(main_logger);
            
            // 遇到 Error 级别及以上时，立刻强制刷新写入磁盘
            spdlog::flush_on(config.flush_level);

        } catch (const spdlog::spdlog_ex& ex) {
            spdlog::error("Failed to initialize logger: {}", ex.what());
        }
    }

}