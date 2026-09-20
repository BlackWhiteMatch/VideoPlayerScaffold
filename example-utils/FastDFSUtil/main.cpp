#include "../../include/VideoPlayerScaffold/utils/FastDFSUtil.h"
#include <iostream>
#include <fstream>
#include <optional>
#include <string>

// 辅助函数：自动创建一个测试用的文本文件
void createSampleFile(const std::string& path, const std::string& content) {
    std::ofstream ofs(path);
    ofs << content;
    ofs.close();
}

int main() {
    std::cout << "===== 开始 FastDFS 基础功能测试 =====" << std::endl;

    // 1. 初始化客户端
    FastDFSUtil::FastDFSSetting setting{{"192.168.204.128:22122"}};
    if (!FastDFSUtil::FastDFSClient::Initialize(setting)) {
        std::cerr << "[错误] FastDFSClient 初始化失败，请检查 Tracker IP 和端口！" << std::endl;
        return -1;
    }
    std::cout << "[1/4] 初始化成功" << std::endl;

    // 2. 准备并上传文件
    const std::string testUploadFilePath = "./test_sample.txt";
    const std::string testContent = "Hello FastDFS, this is a test sample content!";
    createSampleFile(testUploadFilePath, testContent);

    std::optional<std::string> uploadResult = FastDFSUtil::FastDFSClient::UploadFile(testUploadFilePath);
    if (!uploadResult.has_value()) {
        std::cerr << "[错误] 文件上传失败！" << std::endl;
        return -1;
    }
    std::string fileId = uploadResult.value();
    std::cout << "[2/4] 上传成功，File ID: " << fileId << std::endl;

    // 3. 下载文件
    const std::string downloadedFilePath = "./download_sample.txt";
    bool downloadSuccess = FastDFSUtil::FastDFSClient::DownloadFile(fileId, downloadedFilePath);
    if (!downloadSuccess) {
        std::cerr << "[错误] 文件下载失败！" << std::endl;
    } else {
        std::cout << "[3/4] 下载成功，保存至: " << downloadedFilePath << std::endl;
    }

    // 4. 删除文件并清理
    if (FastDFSUtil::FastDFSClient::DeleteFile(fileId)) {
        std::cout << "[4/4] 文件已成功从 FastDFS 删除" << std::endl;
    } else {
        std::cerr << "[警告] 从 FastDFS 删除文件失败" << std::endl;
    }

    std::cout << "===== 测试全部通过！=====" << std::endl;
    return 0;
}
