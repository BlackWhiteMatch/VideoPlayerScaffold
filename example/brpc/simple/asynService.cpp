#include "calculator.pb.h"
#include "../../../include/VideoPlayerScaffold/utils/Logger.h"
#include <brpc/closure_guard.h>
#include <brpc/server.h>
#include <butil/compiler_specific.h>
#include <chrono>
#include <cstdint>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <gflags/gflags.h>
#include <thread>
#include <spdlog/fmt/ostr.h>

// 构建异步服务器
class CalculatetorServiceImpl : public calculator::CalculatorService {
    void Add(
        ::google::protobuf::RpcController* cntl_base,
        const calculator::AddRequest* request,
        calculator::AddResponse* response,
        ::google::protobuf::Closure* done
    )
    override{
        std::thread workThread([=](){
            std::this_thread::sleep_for(std::chrono::seconds(3));
            brpc::ClosureGuard guard(done);
            int32_t num1 = request->num1();
            int32_t num2 = request->num2();
            response->set_result(num1 + num2);
            std::thread::id id = std::this_thread::get_id();
            LOG_INFO("Thread: {} 完成请求处理: {} + {} = {}", id, num1, num2, num1 + num2);
        });
        workThread.detach();
        LOG_INFO("===============================");
    }
};

int main(int argc, char* argv[]){
    google::ParseCommandLineFlags(&argc, &argv, true);

    CalculatetorServiceImpl server_impl;
    
    brpc::Server server;
    
    if(server.AddService(&server_impl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0){
        LOG_ERROR("服务器注册失败！");
        return -1;
    }

    brpc::ServerOptions options;
    if(server.Start(9000, &options) != 0){
        LOG_ERROR("服务器启动失败！");
        return -1;
    }

    LOG_INFO("服务器已启动, 监听端口: 9000");

    server.RunUntilAskedToQuit();

    return 0;
}