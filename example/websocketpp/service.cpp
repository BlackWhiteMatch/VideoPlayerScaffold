#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/frame.hpp>
#include <websocketpp/http/constants.hpp>
#include <websocketpp/logger/levels.hpp>
#include <websocketpp/roles/server_endpoint.hpp>
#include <websocketpp/server.hpp>
#include <iostream>

typedef websocketpp::server<websocketpp::config::asio> websocketsvr_t;

void OnOpen(websocketsvr_t& service, websocketpp::connection_hdl hdl){
    std::cout << "获取成功！" << std::endl;
}

void OnClose(websocketsvr_t& service, websocketpp::connection_hdl hdl){
    std::cout << "连接关闭！" << std::endl;
}

void OnMessage(websocketsvr_t& service, websocketpp::connection_hdl hdl, websocketsvr_t::message_ptr msg){
    std::cout << "接收到消息: " << msg->get_payload() << std::endl;
    service.send(hdl, msg->get_payload(), websocketpp::frame::opcode::text);
}

void OnHttp(websocketsvr_t& service, websocketpp::connection_hdl hdl){
    std::cout << "处理http请求: " << std::endl;
    websocketsvr_t::connection_ptr con = service.get_con_from_hdl(hdl);
    const auto& request = con->get_request();
    std::cout << "mothed: " << request.get_method() << std::endl;
    std::cout << "uri: " << request.get_uri() << std::endl;
    std::cout << "body: " << request.get_body() << std::endl;
    const auto& headers = request.get_headers();
    for(auto& it : headers){
        std::cout << it.first << ": " << it.second << std::endl;
    }
    con->set_status(websocketpp::http::status_code::ok);
    con->set_body("<html><h1>HelloWorld</h1></html>");
    con->append_header("Content-Type", "text/html");
    con->append_header("Access-Control-Allow-Origin", "*");
    con->append_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    con->append_header("Access-Control-Allow-Headers", "Content-Type");
}

int main(){
    websocketsvr_t service;
    service.set_access_channels(websocketpp::log::alevel::none);
    service.init_asio();
    service.set_open_handler(std::bind(&OnOpen, std::ref(service), std::placeholders::_1));
    service.set_close_handler(std::bind(&OnClose, std::ref(service), std::placeholders::_1));
    service.set_message_handler(std::bind(&OnMessage, std::ref(service), std::placeholders::_1, std::placeholders::_2));
    service.set_http_handler(std::bind(&OnHttp, std::ref(service), std::placeholders::_1));
    service.listen(9000);
    service.start_accept();
    service.run();
    return 0;
}