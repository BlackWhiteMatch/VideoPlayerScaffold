#include "../../include/VideoPlayerScaffold/utils/FFmpegUtil.h"
#include "../../include/VideoPlayerScaffold/utils/Logger.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

extern "C"{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/error.h>
    #include <libavcodec/codec_par.h>
    #include <libavcodec/packet.h>
    #include <libavutil/avutil.h>
    #include <libavutil/dict.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/mem.h>
}

using namespace FFmpegUtil;

bool M3U8Parser::ParseString(const std::string& content){
    _headers.clear();
    _segments.clear();
    _is_has_endlist = false;

    // 清理 \r 与 空行
    std::vector<std::string> lines = CleanAndSplit(content);

    if(lines.empty() || lines[0] != "#EXTM3U"){
        LOG_ERROR("解析失败, 文件为非法的M3U8文件, 缺失 #EXTM3U标识符!");
        return false;
    }
    // 逐行解析
    for(size_t i = 0; i < lines.size(); i++){
        const std::string& line = lines[i];

        if(line == "#EXT-X-ENDLIST"){
            _is_has_endlist = true;
            continue;
        }

        if(line.rfind("#EXTINF:", 0) == 0){
            if(i + 1 < lines.size()){
                _segments.emplace_back(std::make_pair(lines[i], lines[i + 1]));
                i++;
            }
            continue;            
        }

        _headers.push_back(line);
    }

    return true;
}

std::string M3U8Parser::Serialize() const{
    std::ostringstream oss;
    for(const auto& header : _headers){
        oss << header << '\n';
    }
    for(const auto& segment : _segments){
        oss << segment.first << '\n';
        oss << segment.second << '\n';
    }
    if(_is_has_endlist){
        oss << "#EXT-X-ENDLIST" << "\n";
    }
    return oss.str();
}

bool M3U8Parser::ApplyBaseURL(const std::string& prefix){
    for(auto& segment : _segments){
        size_t slash = segment.second.find_last_of('/');
        std::string filename = (slash == std::string::npos) ? segment.second : segment.second.substr(slash + 1);
        segment.second = prefix + filename;
    }
    return true;
}

// ----------------------------------------------- HLS -----------------------------------------------
namespace {
    using DemuxContextPtr   = std::unique_ptr<AVFormatContext, void(*)(AVFormatContext*)>;
    using MuxContextPtr     = std::unique_ptr<AVFormatContext, void(*)(AVFormatContext*)>;
    using PacketPtr         = std::unique_ptr<AVPacket, void(*)(AVPacket*)>;
    using DictionaryPtr     = std::unique_ptr<AVDictionary, void(*)(AVDictionary*)>;
}

void HLSSegmenter::SetErrorContext(const int error_code, const std::string& stage_description){
    if(error_code < 0){
        char err_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(error_code, err_buf, sizeof(err_buf));
        _lastErrorContext = stage_description + ": " + std::string(err_buf);
    }
}

bool HLSSegmenter::Remux(
    std::string source,
    std::string target_path,
    ProgressCallback on_progress
){
    // 1.设置输入上下文对象
    AVFormatContext* raw_demux_ctx = nullptr;
    int ret = avformat_open_input(&raw_demux_ctx, source.c_str(), nullptr, nullptr);
    if(ret < 0){
        SetErrorContext(ret, "打开视频流失败");
        return false;
    }
    DemuxContextPtr demux_ctx(raw_demux_ctx, [](AVFormatContext* ctx){
       avformat_close_input(&ctx); 
    });

    ret = avformat_find_stream_info(demux_ctx.get(), nullptr);
    if(ret < 0){
        SetErrorContext(ret, "解析流数据失败");
        return false;
    }

    // 2.初始化目标HLS
    AVFormatContext* raw_mux_ctx = nullptr;
    ret = avformat_alloc_output_context2(&raw_mux_ctx, nullptr, "hls", target_path.c_str());
    if(ret < 0){
        SetErrorContext(ret, "Failed to allocate HLS muxer context");
        return false;
    }
    MuxContextPtr mux_ctx(raw_mux_ctx, [](AVFormatContext* ctx){
        avformat_free_context(ctx);
    });

    // 3.同步基础编码器参数
    for(unsigned int i = 0; i < demux_ctx->nb_streams; i++){
        AVStream* in_stream = demux_ctx->streams[i];

        // 创建一条新的流
        AVStream* out_stream = avformat_new_stream(mux_ctx.get(), nullptr);
        if(!out_stream){
            _lastErrorContext = "创建流失败";
            return false;
        }

        // 把输入的编码参数（如 H264, AAC）直接复制给输出，避免重新编码
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if(ret < 0){
            SetErrorContext(ret, "复制编码器参数失败");
            return -1;   
        }
        out_stream->codecpar->codec_tag = 0;
    }

    // 4.构建分片器字典
    AVDictionary* hls_opts = nullptr;
    av_dict_set_int(&hls_opts, "hls_time", _config.target_segment_duration, 0);
    av_dict_set(&hls_opts, "hls_playlist_type", _config.playlist_type.c_str(), 0);
    if(_config.independent_segment){
        av_dict_set(&hls_opts, "hls_flags", "independent_segments", 0);  
    }
    DictionaryPtr muxer_opts(hls_opts, [](AVDictionary* dict){
        av_dict_free(&dict);
    });

    // 5.写入分片头部协议
    AVDictionary* opts_transfer_ptr = muxer_opts.release();
    ret = avformat_write_header(mux_ctx.get(), &opts_transfer_ptr);
    if(ret < 0){
        SetErrorContext(ret, "写入分片头部协议失败");
        return false;
    }

    // 6.流式数据包读取与时间基重采样
    PacketPtr packet(av_packet_alloc(), [](AVPacket* packet){
        av_packet_free(&packet);
    });

    const uint64_t stream_total_duration_us = demux_ctx->duration;

    while(av_read_frame(demux_ctx.get(), packet.get()) >= 0){
        AVStream* source_stream = demux_ctx->streams[packet->stream_index];
        AVStream* target_stream = mux_ctx->streams[packet->stream_index];

        // 规范化缺失的显示时间戳
        if (packet->pts == AV_NOPTS_VALUE) {
            packet->pts = av_rescale_q(0, AV_TIME_BASE_Q, source_stream->time_base);
            packet->dts = packet->pts;
        }

        // 计算进度比例
        if (on_progress && stream_total_duration_us > 0 && packet->pts != AV_NOPTS_VALUE) {
            int64_t current_pts_us = av_rescale_q(packet->pts, source_stream->time_base, AV_TIME_BASE_Q);
            double progress_ratio = static_cast<double>(current_pts_us) / stream_total_duration_us;
            on_progress(progress_ratio > 1.0 ? 1.0 : progress_ratio);
        }

        // 转换时间基：Source TimeBase -> Destination TimeBase
        av_packet_rescale_ts(packet.get(), source_stream->time_base, target_stream->time_base);
        packet->pos = -1;

        ret = av_interleaved_write_frame(mux_ctx.get(), packet.get());
        if (ret < 0) {
            SetErrorContext(ret, "交错帧写入失败");
            av_packet_unref(packet.get());
            return false;
        }

        av_packet_unref(packet.get());
    }

    // 7.封包收尾
    ret = av_write_trailer(mux_ctx.get());
    if(ret < 0){
        SetErrorContext(ret, "封包收尾写入失败");
        return false;
    }
    
    if(on_progress){
        on_progress(1.0);
    }

    return true;
}

