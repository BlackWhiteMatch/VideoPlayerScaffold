#include "../../../include/VideoPlayerScaffold/utils/Logger.h"
#include "calculator.pb.h"
#include <brpc/callback.h>
#include <brpc/channel.h>
#include <brpc/closure_guard.h>
#include <brpc/controller.h>
#include <brpc/protocol.h>
#include <functional>
#include <google/protobuf/stubs/callback.h>
#include <unistd.h>

using callback_t = std::function<void()>;
struct callbackHelper{
    callback_t callback;
};
void rpc_callback(callbackHelper helper){
    helper.callback();
}

int main(){
    brpc::Channel channel;
    brpc::ChannelOptions opts;
    opts.timeout_ms = 4000;
    int init_result = channel.Init("0.0.0.0:9000", &opts);
    if(init_result != 0){
        LOG_ERROR("Channel init 失败!");
        return -1;
    }

    calculator::CalculatorService_Stub stub(&channel);
    calculator::AddRequest* req = new calculator::AddRequest();
    req->set_num1(123);
    req->set_num2(456);
    brpc::Controller* ctl = new brpc::Controller();
    calculator::AddResponse* resp = new calculator::AddResponse();
    
    // 异步闭包
    callbackHelper helper;
    helper.callback = [&](){
        if(!ctl->Failed()){
            LOG_INFO("brpc调用成功!");
            LOG_INFO("{} + {} = {}", req->num1(), req->num2(), req->num1() + req->num2());
        }else{
            LOG_WARN("brpc调用失败, 错误码: {}", ctl->ErrorCode());
        }
    };
    google::protobuf::Closure* done = brpc::NewCallback(rpc_callback, helper);

    stub.Add(ctl, req, resp, done);

    LOG_INFO("============等待结果============");
    sleep(5);
    return 0;
}