#include <httplib.h>

void HelloWorld(const httplib::Request& req, httplib::Response& rsp){
    std::cout << req.method << std::endl;
    std::cout << req.path << std::endl;
    std::cout << req.body << std::endl;
    for(auto& e : req.headers){
        std::cout << e.first << " = " << e.second << std::endl;
    }
    std::string rpsStr = "<html><body><h1>HelloWorld</h1></body></html>";
    rsp.set_content(rpsStr, "text/html");
    rsp.status = 200;
}

int main(){
    httplib::Server server;
    server.Get("/hi", HelloWorld);
    server.Get(R"(/number/(\d+))", [](const httplib::Request& req, httplib::Response& rsp){
        std::cout << req.method << std::endl;
        std::cout << req.path << std::endl;
        std::cout << req.body << std::endl;
        for(auto& e : req.headers){
            std::cout << e.first << " = " << e.second << std::endl;
        }
        std::string rpsStr = "<html><body><h1>NumberXXX</h1></body></html>";
        rsp.set_content(rpsStr, "text/html");
        rsp.status = 200;
    });
    server.listen("0.0.0.0", 9000);
    return 0;
}