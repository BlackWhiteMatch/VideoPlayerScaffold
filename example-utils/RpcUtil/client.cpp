#include "calculator.pb.h"
#include "../../include/VideoPlayerScaffold/utils/RpcUtil.h"
#include "../../include/VideoPlayerScaffold/utils/Logger.h"
#include <brpc/controller.h>
#include <brpc/server.h>
#include <memory>

int main(){
    RpcUtil::ServiceManager manager;
    std::string service_name("calculator_service");

    manager.watch(service_name);
    manager.addNode(service_name, "192.168.204.128:9000");    

    std::shared_ptr<RpcUtil::Channel> channel_node = manager.getNode(service_name);
    if (!channel_node) {
        LOG_ERROR("获取服务节点失败！");
        return -1;
    }
    calculator::CalculatorService_Stub stub(channel_node->raw());

    for(int i = 1; i <= 3; i++){
        auto cntl = new brpc::Controller();
        auto request = new calculator::AddRequest();
        auto response = new calculator::AddResponse();

        int num1 = i * 10;
        int num2 = i * 5;

        request->set_num1(num1);
        request->set_num2(num2);

        auto done = RpcUtil::ClosureFactory::create([cntl, request, response, i](){
            // 使用 unique_ptr 保证在 Lambda 退出时自动 delete 内存
            std::unique_ptr<brpc::Controller> cntl_guard(cntl);
            std::unique_ptr<calculator::AddRequest> req_guard(request);
            std::unique_ptr<calculator::AddResponse> rsp_guard(response);

            if (cntl_guard->Failed()) {
                LOG_ERROR("第 {} 次 RPC 请求失败: {}", i, cntl_guard->ErrorText());
            } else {
                LOG_INFO("第 {} 次 RPC 异步响应成功！{} + {} = {}", 
                    i, 
                    req_guard->num1(), 
                    req_guard->num2(), 
                    rsp_guard->result()
                );
            }
        });

        LOG_INFO("发起第 {} 次计算请求: {} + {} ...", i, num1, num2);
        
        // 7. 发起异步 RPC 调用（立即返回，非阻塞）
        stub.Add(cntl, request, response, done);
    }

    LOG_INFO("所有异步请求已发出，等待服务端响应...");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
}