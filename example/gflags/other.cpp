#include <gflags/gflags.h>
#include <iostream>
#include "other.h"

DECLARE_bool(reuse_port);
DECLARE_string(listen_ip);
DECLARE_int32(listen_port);
DECLARE_double(pi);

void printFlags(){
    std::cout << "printFlags():" << std::endl;
    std::cout << "reuse_port: " << FLAGS_reuse_port << std::endl;
    std::cout << "listen_ip: " << FLAGS_listen_ip << std::endl;
    std::cout << "listen_port: " << FLAGS_listen_port << std::endl;
    std::cout << "pi: " << FLAGS_pi << std::endl;
}