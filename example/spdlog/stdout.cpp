#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <iostream>

int main(int argc, char* argv[])
{
    // 控制台输出
    // auto logger = spdlog::stdout_color_mt("stdout");
    // 文件输出
    // auto logger = spdlog::basic_logger_mt("basic_logger", "basic.log");
    // 循环文件输出
    auto logger = spdlog::rotating_logger_mt("rotating_logger", "rotating.log", 1024, 3);
    logger->set_level(spdlog::level::debug);
    logger->set_pattern("[%H:%M:%S][%-7l]: %v");

    for(int i = 1; i <= 10000; i++){
        logger->info("hello world {}", i);
    }

    // logger->info("hello world");
    // logger->error("error world");
    // logger->warn("warn world");
    // logger->debug("debug world");
    // logger->trace("trace world");
    
    return 0;
}