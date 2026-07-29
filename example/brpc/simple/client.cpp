#include "calculator.pb.h"
#include "../../../include/VideoPlayerScaffold/utils/Logger.h"
#include <brpc/channel.h>
#include <brpc/controller.h>
#include <brpc/options.pb.h>
#include <cstdint>

int main(){
    brpc::Channel channel;
    brpc::ChannelOptions opts;
    opts.protocol = brpc::PROTOCOL_BAIDU_STD;
    opts.timeout_ms = 3000;

    int init_result = channel.Init("0.0.0.0:9000", &opts);
    if(init_result != 0){
        LOG_ERROR("Channel初始化失败, 错误码: {}", init_result);
        return -1;
    }

    calculator::CalculatorService_Stub stub(&channel);
    
    calculator::AddRequest req;
    calculator::AddResponse resp;
    brpc::Controller ctrl;

    int32_t n1 = 123;
    int32_t n2 = 456;
    req.set_num1(n1);
    req.set_num2(n2);

    stub.Add(&ctrl, &req, &resp, nullptr);

    if(!ctrl.Failed()){
        LOG_INFO("brpc调用成功!");
        LOG_INFO("{} + {} = {}", req.num1(), req.num2(), req.num1() + req.num2());
    }else{
        LOG_WARN("brpc调用失败, 错误码: {}", ctrl.ErrorCode());
        return -1;
    }
    
    return 0;
}