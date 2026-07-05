#include "../../include/VideoPlayerScaffold/utils/Logger.h"
#include <gflags/gflags.h>

using utils::gLogger;

DEFINE_bool(use_async, true, "是否异异步输出日志");
DEFINE_bool(enable_console, true, "是否在控制台输出日志");
DEFINE_bool(enable_file, true, "是否在文件输出日志");
DEFINE_string(level, "info", "日志级别");

int main(int argc, char* argv[]){
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    utils::LoggerConfig config;
    config.use_async = FLAGS_use_async;
    config.enable_console = FLAGS_enable_console;
    config.enable_file = FLAGS_enable_file;
    config.level = spdlog::level::from_str(FLAGS_level);

    gLogger.Init(config);
    LOG_INFO("这是一条info日志");
    LOG_ERROR("这是一条error日志");
    LOG_WARN("这是一条warn日志");
    LOG_DEBUG("这是一条debug日志");
    LOG_TRACE("这是一条trace日志");
    LOG_CRITICAL("这是一条citical日志");
    
    return 0;
}

