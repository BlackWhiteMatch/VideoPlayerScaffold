#pragma once
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace FFmpegUtil {
class M3U8Parser {
    // 单元信息存储格式
    using SegmentPair = std::pair<std::string, std::string>; 
public:
    // 解析M3U8文本内容
    bool ParseString(const std::string& content);
    // 重新组装M3U8文本 
    std::string Serialize () const;
    // 替换全部视频的URL
    bool ApplyBaseURL(const std::string& prefix);

    // 获取 headers 与 segments
    std::vector<std::string>& GetHeaders(){ return _headers;}
    std::vector<SegmentPair>& GetSegments() {return _segments;}
private:
    std::vector<std::string> _headers;
    std::vector<SegmentPair> _segments;
    bool _is_has_endlist = false;

    std::vector<std::string> CleanAndSplit(const std::string& text){
        std::vector<std::string> result;
        std::istringstream iss(text);
        std::string line;
        while(std::getline(iss, line)){
            if(!line.empty() && line.back() == '\r'){
                line.pop_back();
            }
            if(!line.empty()){
                result.push_back(line);
            }
        }
        return result;
    }
};

// ----------------------------------------------- HLS -----------------------------------------------
struct HLSSegmenterConfig {
    uint32_t target_segment_duration = 5;
    std::string playlist_type = "vod";
    bool independent_segment = true;
};

class HLSSegmenter {
public:
    using ProgressCallback = std::function<void(double ratio)>;
    explicit HLSSegmenter(HLSSegmenterConfig config = {}) : _config(std::move(config)){}

    HLSSegmenter(const HLSSegmenter&) = delete;
    HLSSegmenter& operator=(const HLSSegmenter&) = delete;
    HLSSegmenter(HLSSegmenter&&) noexcept = default;
    HLSSegmenter& operator=(HLSSegmenter&&) noexcept = default;

    bool Remux(
        std::string source,
        std::string target_path,
        ProgressCallback on_progress = nullptr
    );
    std::string GetErrorContext() const { return _lastErrorContext;}
private:
    HLSSegmenterConfig _config;
    std::string _lastErrorContext;
    void SetErrorContext(const int error_code, const std::string& stage_description);
};
}
