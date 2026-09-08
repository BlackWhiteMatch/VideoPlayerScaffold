#include "../../include/VideoPlayerScaffold/utils/FFmpegUtil.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace FFmpegUtil;

bool read_file_to_string(const std::string& path, std::string& out_content) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;
    std::ostringstream ss;
    ss << file.rdbuf();
    out_content = ss.str();
    return true;
}

int main(int argc, char* argv[]) {
    if(argc != 3){
        std::cout << "Usage: ./main <input_filename> <output_filename>" << std::endl << std::endl;
        return -1;
    }
    std::string input_video = argv[1];
    // output_playlist.m3u8
    std::string output_m3u8 = argv[2];
    std::string cdn_m3u8 = "output_cdn_playlist.m3u8";

    // 1. 测试切片转封装
    std::cout << "========== [阶段 1: 测试 HLSSegmenter] ==========\n";
    HLSSegmenterConfig config;
    config.target_segment_duration = 10; // 10 秒一个切片
    config.playlist_type = "vod";

    HLSSegmenter segmenter(config);

    bool remux_ok = segmenter.Remux(input_video, output_m3u8, [](double ratio) {
        std::cout << "\r[转封装进度]: " << std::fixed << std::setprecision(1) 
                  << ratio * 100.0 << "%" << std::flush;
    });

    if (!remux_ok) {
        std::cerr << "\n转封装失败！错误: " << segmenter.GetErrorContext() << std::endl;
        return -1;
    }
    std::cout << "\n转封装切片测试通过！\n\n";

    // 2. 测试 M3U8 解析与前缀拼接
    std::cout << "========== [阶段 2: 测试 M3U8Parser] ==========\n";

    // 先调用工具函数将本地文件读入内存 string
    std::string m3u8_content;
    if (!read_file_to_string(output_m3u8, m3u8_content)) {
        std::cerr << "读取生成的 M3U8 文件失败！\n";
        return -1;
    }

    // 将真实的文本内容喂给 ParseString
    M3U8Parser parser;
    if (!parser.ParseString(m3u8_content)) {
        std::cerr << "解析生成的 M3U8 文本内容失败！\n";
        return -1;
    }

    std::cout << "M3U8 解析成功！切片详情如下:\n";
    for (size_t i = 0; i < parser.GetSegments().size(); ++i) {
        std::cout << "  - 分片 [" << i << "]: " 
                  << parser.GetSegments()[i].first << " -> " 
                  << parser.GetSegments()[i].second << "\n";
    }

    // 3. 测试批量注入 CDN 前缀并写入新文件
    std::string fake_cdn = "http://127.0.0.1:8080/static/";
    parser.ApplyBaseURL(fake_cdn);

    // 接收序列化结果并将其写回目标文件
    std::string modified_content = parser.Serialize();

    std::ofstream out_file(cdn_m3u8, std::ios::binary | std::ios::trunc);
    if (!out_file.is_open()) {
        std::cerr << "写入 CDN M3U8 文件失败！\n";
        return -1;
    }
    out_file << modified_content;
    out_file.close();

    std::cout << "\n已成功生成带 CDN 前缀的 " << cdn_m3u8 << " 文件！\n";

    return 0;
}