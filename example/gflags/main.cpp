#include <gflags/gflags.h>
#include <iostream>
#include "other.h"

DEFINE_bool(reuse_port, true, "启动地址复用选项");
DEFINE_string(listen_ip, "0.0.0.0", "监听IP地址");
DEFINE_int32(listen_port, 9090, "监听端口");
DEFINE_double(pi, 3.1415926, "圆周率");

int main(int argc, char* argv[]){
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::cout << "reuse_port: " << FLAGS_reuse_port << std::endl;
    std::cout << "listen_ip: " << FLAGS_listen_ip << std::endl;
    std::cout << "listen_port: " << FLAGS_listen_port << std::endl;
    std::cout << "pi: " << FLAGS_pi << std::endl;

    printFlags();
    return 0;
}