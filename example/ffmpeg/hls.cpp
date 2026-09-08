#include <iostream>
#include <string>

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

// 辅助函数 返回报错字符
std::string parseAVError(int err_num){
    if(err_num < 0){
        char err_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(err_num, err_buf, sizeof(err_buf));
        return std::string(err_buf);
    }
    return std::string("");
}

int main(int argc, char* argv[]){
    if(argc < 3){
        std::cout << "Usage: ./hls <input_file> <output_m3u8>" << std::endl;
        return -1;
    }
    
    const char* input_file = argv[1];
    const char* output_file = argv[2];

    std::cout << "input_file: " << input_file << std::endl;
    std::cout << "output_file: " << output_file << std::endl;
    
    // 打开文件
    AVFormatContext* in_fmt_ctx = nullptr;
    int ret = avformat_open_input(&in_fmt_ctx, input_file, nullptr, nullptr);
    if(ret < 0){
        std::cerr << "打开文件 " << input_file << " 失败！原因: " << parseAVError(ret) << std::endl;
        return -1;
    }
    // 解析数据
    ret = avformat_find_stream_info(in_fmt_ctx, nullptr);
    if(ret < 0){
        std::cerr << "解析数据失败！原因: " << parseAVError(ret) << std::endl;
        avformat_close_input(&in_fmt_ctx);
        return -1;
    }
    //打印解析详情
    av_dump_format(in_fmt_ctx, 0, input_file, 0);

    // 创建输出上下文对象，并且传入信息
    AVFormatContext* out_fmt_ctx = nullptr;
    ret = avformat_alloc_output_context2(&out_fmt_ctx, nullptr, "hls", output_file);
    if(ret < 0){
        std::cerr << "创建上下文对象失败! 原因: " << parseAVError(ret) << std::endl;
        avformat_close_input(&in_fmt_ctx);
        return -1;
    }
    // 遍历输入流，为输出上下文创建一摸一样的轨道，并且拷贝编码信息
    for(unsigned int i = 0; i < in_fmt_ctx->nb_streams; i++){
        AVStream* in_stream = in_fmt_ctx->streams[i];

        // 创建一条新的流
        AVStream* out_stream = avformat_new_stream(out_fmt_ctx, nullptr);
        if(!out_stream){
            std::cerr << "创建流失败!" << std::endl;
            avformat_close_input(&in_fmt_ctx);
            avformat_free_context(out_fmt_ctx);
            return -1;
        }

        // 把输入的编码参数（如 H264, AAC）直接复制给输出，避免重新编码
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if(ret < 0){
            std::cerr << "复制编码器参数失败! 原因: " << parseAVError(ret);
            avformat_close_input(&in_fmt_ctx);
            avformat_free_context(out_fmt_ctx);
            return -1;   
        }
        out_stream->codecpar->codec_tag = 0;
    }

    // 配置HLS
    AVDictionary* hls_opts = nullptr;
    av_dict_set_int(&hls_opts, "hls_time", 5, 0);
    av_dict_set(&hls_opts, "hls_playlist_type", "vod", 0);
    av_dict_set(&hls_opts, "hls_flags", "independent_segments", 0);
    av_dict_set(&hls_opts, "hls_base_url", "http://192.168.204.1:9000/video/", 0);

    // 写入容器头部信息（此时会在磁盘创建 .m3u8 和第一个 .ts 文件）
    ret = avformat_write_header(out_fmt_ctx, &hls_opts);
    if(ret < 0){
        std::cerr << "写入头部信息失败! 原因: " << parseAVError(ret) << std::endl;
        av_dict_free(&hls_opts);
        avformat_close_input(&in_fmt_ctx);
        avformat_free_context(out_fmt_ctx);
        return -1;
    }
    av_dict_free(&hls_opts);

    AVPacket* packet = av_packet_alloc();
    while(av_read_frame(in_fmt_ctx, packet) >= 0){
        AVStream* in_stream = in_fmt_ctx->streams[packet->stream_index];
        AVStream* out_stream = out_fmt_ctx->streams[packet->stream_index];
        if(packet->pts == AV_NOPTS_VALUE){
            packet->pts = av_rescale_q(0, AV_TIME_BASE_Q, in_stream->time_base);
            packet->dts = packet->pts;
        }

        av_packet_rescale_ts(packet, in_stream->time_base, out_stream->time_base);
        packet->pos = -1;
        
        ret = av_interleaved_write_frame(out_fmt_ctx, packet);
        if(ret < 0){
            std::cerr << "写入数据包失败! 原因: " << parseAVError(ret);
            av_packet_unref(packet);
            break;
        }

        av_packet_unref(packet);
    }

    av_write_trailer(out_fmt_ctx);

    av_packet_free(&packet);
    avformat_free_context(out_fmt_ctx);
    avformat_close_input(&in_fmt_ctx);

    std::cout << "HLS 切片转封装完成！\n";
    
    return 0;
}