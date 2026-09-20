#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <mutex>

namespace FastDFSUtil {
    struct FastDFSSetting {
        std::vector<std::string> trackerServers;        // Tracker队列
        int connectionTimeoutSeconds = 30;              // 最大连接时间
        int neworkTimeoutSeconds = 30;                  // 最大网络通信时间
        bool useConnectionPool = true;                  // 是否使用连接池
        int connectionPoolMaxIdleTimeSeconds = 3600;    // 连接池空闲最大保存时间
    };

    class FastDFSClient {
    public:
        // 禁止实例化，全部为静态工具函数
        FastDFSClient() = delete;
        ~FastDFSClient() = delete;

        // 初始化函数
        static bool Initialize(const FastDFSSetting& setting);

        static void Destory();

        // 上传文件导磁盘
        static std::optional<std::string> UploadFile(const std::string& filePath);
        // 上传到缓冲区
        static std::optional<std::string> UploadBuffer(
            const uint8_t* bufferData,
            size_t dataSize,
            const std::string& fileExtension = ""
        );
        static std::optional<std::string> UploadBuffer(
            const std::string& bufferString,
            const std::string& fileExtension = ""
        );

        // 下载文件到本地
        static bool DownloadFile(
            const std::string fileIdentifier,
            const std::string& targetLocalFilePath
        );
        // 下载到缓冲区
        static bool DownloadBuffer(
            const std::string& fileIdentifier,
            std::string& outputBuffer
        );

        // 删除文件
        static bool DeleteFile(const std::string& fileIdentifier);

    private:
        static bool _isInitialized;
        static std::mutex _initializationMutex;
    };

}
