#include <algorithm>
#include <cstdint>
#include <fastcommon/connection_pool.h>
#include <functional>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>

extern "C" {
#include <fastcommon/logger.h>
#include <fastdfs/fdfs_client.h>
}

int main(){
    // 声明并初始化日志模块
    g_log_context.log_level = LOG_ERR;
    log_init();

    // 声明配置文件，并进行初始化配置
    const char* configurationFilePath = "./client.conf";
    int initializeResult = fdfs_client_init(configurationFilePath);
    if(initializeResult != 0){
        std::cerr << "[错误] 初始化 FastDFS 客户端配置失败: " << STRERROR(initializeResult) << std::endl;
        return -1;
    }
    std::cout << "[1] FastDFS 客户端全局初始化成功！" << std::endl;

    // 准备临时测试文件
    const std::string testUploadFilePath = "./test_sample.txt";
    {
        std::ofstream outputTestFile(testUploadFilePath);
        outputTestFile << "Hello FastDFS! This is a test for VideoPlayerScaffold storage pipeline." << std::endl;
    }

    // 连接Tracker服务器
    ConnectionInfo* trackerServerConnection = tracker_get_connection();
    if(trackerServerConnection == nullptr){
        std::cerr << "[错误] 连接 Tracker 服务器失败，请检查 IP、端口与服务状态！" << std::endl;
        fdfs_client_destroy();
        return -1;
    }
    std::cout << "[2] 成功连接至 Tracker 调度服务器！" << std::endl;

    // 上传文件到Storage节点
    constexpr size_t maxFileIdentifierLength = 256;
    char generatedFileIdentifier[maxFileIdentifierLength] = {0};
    int uploadResult = storage_upload_by_filename1(
        trackerServerConnection,
        nullptr,
        0,
        testUploadFilePath.c_str(),
        nullptr,
        nullptr,
        0,
        nullptr,
        generatedFileIdentifier
    );
    if(uploadResult != 0){
        std::cerr << "[错误] 文件上传失败: " << STRERROR(uploadResult) << std::endl;
        tracker_close_connection_ex(trackerServerConnection, false);
        fdfs_client_destroy();
        return -1;
    }
    std::cout << "[3] 文件上传成功！FastDFS FileID: " << generatedFileIdentifier << std::endl;

    // 下载刚传的文件到本地，成为新文件
    const std::string downloadedFilePath = "./download_sample.txt";
    int64_t downloadedFileSizeBytes = 0;
    int downloadResult = storage_download_file_to_file1(
        trackerServerConnection,
        nullptr,
        generatedFileIdentifier,
        downloadedFilePath.c_str(),
        &downloadedFileSizeBytes
    );
    if(downloadResult != 0){
        std::cerr << "[错误] 文件下载失败: " << STRERROR(downloadResult) << std::endl;
    }else{
        std::cout << "[4] 文件下载成功！保存到: " << downloadedFilePath
                          << " (大小: " << downloadedFileSizeBytes << " 字节)" << std::endl;
    }

    // 清理测试文件
    int deleteResult = storage_delete_file1(
        trackerServerConnection,
        nullptr,
        generatedFileIdentifier
    );
    if(deleteResult != 0){
        std::cerr << "[错误] 删除文件失败: " << STRERROR(deleteResult) << std::endl;
    }else{
        std::cout << "[5] 远端 FastDFS 文件删除成功！" << std::endl;
    }

    // 清理资源，关闭 Tracker 连接，并且清理客户端配置
    tracker_close_connection_ex(trackerServerConnection, false);
    fdfs_client_destroy();
    std::cout << "[6] 资源已释放完毕。" << std::endl;

    return 0;
}
