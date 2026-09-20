#include "../../include/VideoPlayerScaffold/utils/FastDFSUtil.h"
#include <cstddef>
#include <cstdint>
#include <fastcommon/common_define.h>
#include <fastdfs/storage_client.h>
#include <mutex>
#include <optional>
#include <sstream>

extern "C" {
    #include <fastcommon/connection_pool.h>
    #include <fastcommon/logger.h>
    #include <fastdfs/fdfs_client.h>
    #include <fastdfs/tracker_types.h>
}

namespace FastDFSUtil {
    bool FastDFSClient::_isInitialized = false;
    std::mutex FastDFSClient::_initializationMutex;
    constexpr size_t FDFS_FILE_ID_MAX_LEN = 256;

    class TrackerConnectionGuard {
    public:
        explicit TrackerConnectionGuard(ConnectionInfo* trackerServerConnection)
            : _trackerServerConnection(trackerServerConnection) {}

        ~TrackerConnectionGuard() {
            if(_trackerServerConnection != nullptr){
                tracker_close_connection_ex(_trackerServerConnection, false);
            }
        }

        ConnectionInfo* GetConnection() const { return _trackerServerConnection; }

        bool IsValid() const { return _trackerServerConnection != nullptr; }

        // 禁止拷贝
        TrackerConnectionGuard(const TrackerConnectionGuard&) = delete;
        TrackerConnectionGuard& operator=(const TrackerConnectionGuard&) = delete;
    private:
        ConnectionInfo* _trackerServerConnection = nullptr;
    };

    bool FastDFSClient::Initialize(const FastDFSSetting &setting){
        std::lock_guard<std::mutex> lock(_initializationMutex);
        if(_isInitialized){
            std::cout << "FastDFSClient已经初始化过了，请勿重新初始化！" << std::endl;
            return true;
        }

        // 初始化自身logger
        g_log_context.log_level = LOG_ERR;
        log_init();

        // 拼接配置
        std::stringstream configurationStream;
        for(auto& address : setting.trackerServers){
            configurationStream << "tracker_server = " << address << "\n";
        }
        configurationStream << "connect_timeout = " << setting.connectionTimeoutSeconds << "\n";
        configurationStream << "network_timeout = " << setting.neworkTimeoutSeconds << "\n";
        configurationStream << "use_connection_pool" << (setting.useConnectionPool ? "true" : "false") << "\n";
        configurationStream << "connection_pool_max_idle_time = " << setting.connectionPoolMaxIdleTimeSeconds << "\n";

        std::string configurationContent = configurationStream.str();
        int initializationResult = fdfs_client_init_from_buffer(configurationContent.c_str());
        if(initializationResult) {
            std::cerr << "FastDistributedFileSystemClient::Initialize 失败，原因: " << STRERROR(initializationResult) << std::endl;
            return false;
        }
        _isInitialized = true;
        std::cout << "FastDistributedFileSystemClient 全局上下文配置初始化成功" << std::endl;
        return true;
    }

    void FastDFSClient::Destory(){
        std::lock_guard<std::mutex> lock(_initializationMutex);
        if(!_isInitialized){
            return;
        }
        fdfs_client_destroy();
        _isInitialized = false;
        std::cout << "FastDistributedFileSystemClient 全局资源已完全释放" << std::endl;
    }

    std::optional<std::string> FastDFSClient::UploadFile(const std::string& filePath){
        if(!_isInitialized){
            std::cerr << "FastDistributedFileSystemClient::UploadFile 失败: 模块尚未完成全局初始化" << std::endl;
            return std::nullopt;
        }

        TrackerConnectionGuard connectionGuard(tracker_get_connection());
        if(!connectionGuard.IsValid()){
            std::cerr << "FastDistributedFileSystemClient::UploadFile 失败: 无法连接至任何 Tracker 服务器" << std::endl;
            return std::nullopt;
        }

        char generatedFileIdentifier[FDFS_FILE_ID_MAX_LEN + 1] = {0};
        int uploadResult = storage_upload_by_filename1(
            connectionGuard.GetConnection(),
            nullptr,
            0,
            filePath.c_str(),
            nullptr,
            nullptr,
            0,
            nullptr,
            generatedFileIdentifier
        );

        if(uploadResult != 0){
            std::cerr
            << "FastDistributedFileSystemClient::UploadFile 失败，文件路径: [" << filePath
            << "], 错误: "<< STRERROR(uploadResult) << std::endl;
            return std::nullopt;
        }

        return std::string(generatedFileIdentifier);
    }

    std::optional<std::string> FastDFSClient::UploadBuffer(
        const uint8_t* bufferData,
        size_t dataSize,
        const std::string& fileExtension
    ){
        if(!_isInitialized){
            std::cerr << "FastDistributedFileSystemClient::UploadFile 失败: 模块尚未完成全局初始化" << std::endl;
            return std::nullopt;
        }

        if(bufferData == nullptr || dataSize == 0){
            std::cerr << "FastDistributedFileSystemClient::UploadBuffer 失败: 传入的数据缓冲区为空或长度为零" << std::endl;
            return std::nullopt;
        }

        TrackerConnectionGuard connectionGuard(tracker_get_connection());
        if(!connectionGuard.IsValid()){
            std::cerr << "FastDistributedFileSystemClient::UploadFile 失败: 无法连接至任何 Tracker 服务器" << std::endl;
            return std::nullopt;
        }

        char generatedFileIdentifier[FDFS_FILE_ID_MAX_LEN + 1] = {0};
        const char* extensionPointer = fileExtension.empty()? nullptr : fileExtension.c_str();

        int uploadResult = storage_upload_by_filebuff1(
            connectionGuard.GetConnection(),
            nullptr,
            0,
            reinterpret_cast<const char*>(bufferData),
            dataSize,
            extensionPointer,
            nullptr,
            0,
            nullptr,
            generatedFileIdentifier
        );

        if (uploadResult != 0) {
            std::cerr
                << "FastDistributedFileSystemClient::UploadBuffer 失败，大小: " << dataSize
                << " 字节，错误: " << STRERROR(uploadResult) << std::endl;
            return std::nullopt;
        }

        return std::string(generatedFileIdentifier);
    }

    std::optional<std::string> FastDFSClient::UploadBuffer(
        const std::string& bufferString,
        const std::string& fileExtension
    ){
        return UploadBuffer(
            reinterpret_cast<const uint8_t*>(bufferString.data()),
            bufferString.size(),
            fileExtension
        );
    }

    bool FastDFSClient::DownloadFile(
        const std::string fileIdentifier,
        const std::string& targetLocalFilePath
    ){
        if(!_isInitialized){
            std::cerr << "FastDistributedFileSystemClient::UploadFile 失败: 模块尚未完成全局初始化" << std::endl;
            return false;
        }

        TrackerConnectionGuard connectionGuard(tracker_get_connection());
        if(!connectionGuard.IsValid()){
            std::cerr << "FastDistributedFileSystemClient::UploadFile 失败: 无法连接至任何 Tracker 服务器" << std::endl;
            return false;
        }

        int64_t downloadedFileSizeBytes = 0;
        int downloadResult = storage_download_file_to_file1(
            connectionGuard.GetConnection(),
            nullptr,
            fileIdentifier.c_str(),
            targetLocalFilePath.c_str(),
            &downloadedFileSizeBytes
        );

        if(downloadResult != 0){
            std::cerr
                << "FastDistributedFileSystemClient::DownloadFile 失败，FileID: ["
                <<fileIdentifier << "], 目标路径: [" << targetLocalFilePath << "], 错误: " << STRERROR(downloadResult) << std::endl;
            return false;
        }
        return true;
    }

    bool FastDFSClient::DownloadBuffer(
        const std::string& fileIdentifier,
        std::string& outputBuffer
    ){
        if(!_isInitialized){
            std::cerr << "FastDistributedFileSystemClient::UploadFile 失败: 模块尚未完成全局初始化" << std::endl;
            return false;
        }

        TrackerConnectionGuard connectionGuard(tracker_get_connection());
        if(!connectionGuard.IsValid()){
            std::cerr << "FastDistributedFileSystemClient::UploadFile 失败: 无法连接至任何 Tracker 服务器" << std::endl;
            return false;
        }

        char* allocateBufferPointer = nullptr;
        int64_t downloadedFileBytes = 0;
        int downloadResult = storage_download_file1(
            connectionGuard.GetConnection(),
            nullptr,
            fileIdentifier.c_str(),
            &allocateBufferPointer,
            &downloadedFileBytes
        );

        if(downloadResult != 0){
            std::cerr
                << "FastDistributedFileSystemClient::DownloadBuffer 失败，FileID: ["
                << fileIdentifier << "], 错误: " << STRERROR(downloadResult) << std::endl;
            return false;
        }

        // 放入缓冲区之后就释放
        if(allocateBufferPointer != nullptr && downloadedFileBytes > 0){
            outputBuffer.assign(allocateBufferPointer, static_cast<size_t>(downloadedFileBytes));
            free(allocateBufferPointer);
        }else{
            outputBuffer.clear();
        }

        return true;
    }

    bool FastDFSClient::DeleteFile(const std::string& fileIdentifier){
        if(!_isInitialized){
            std::cerr << "FastDistributedFileSystemClient::UploadFile 失败: 模块尚未完成全局初始化" << std::endl;
            return false;
        }

        TrackerConnectionGuard connectionGuard(tracker_get_connection());
        if(!connectionGuard.IsValid()){
            std::cerr << "FastDistributedFileSystemClient::UploadFile 失败: 无法连接至任何 Tracker 服务器" << std::endl;
            return false;
        }

        int deleteResult = storage_delete_file1(
            connectionGuard.GetConnection(),
            nullptr,
            fileIdentifier.c_str()
        );
        if(deleteResult != 0){
            std::cerr
                << "FastDistributedFileSystemClient::DeleteFile 失败，FileID: ["
                << fileIdentifier <<"], 错误: " << STRERROR(deleteResult) << std::endl;
            return false;
        }
        return true;
    }
}
