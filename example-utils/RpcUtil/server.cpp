#include "calculator.pb.h"
#include "../../include/VideoPlayerScaffold/utils/RpcUtil.h"
#include "../../include/VideoPlayerScaffold/utils/Logger.h"
#include <brpc/server.h>

class CalculatorImpl : public calculator::CalculatorService {
    void Add(
        ::google::protobuf::RpcController* cntl_base,
        const calculator::AddRequest* request,
        calculator::AddResponse* response,
        ::google::protobuf::Closure* done
    )
    override{
        brpc::ClosureGuard guard(done);
        
        int32_t num1 = request->num1();
        int32_t num2 = request->num2();
        response->set_result(num1 + num2);
        LOG_INFO("处理请求: {} + {} = {}", num1, num2, num1 + num2);
    }
};

int main(){
    auto* service = new CalculatorImpl();
    auto server = RpcUtil::RpcServerFactory::create(9000, service);
    if (!server) {
        LOG_ERROR("Failed to start server!");
        return -1;
    }

    LOG_INFO("Server is running on port 9000. Press Ctrl+C to quit.");

    server->RunUntilAskedToQuit();

    return 0;
}