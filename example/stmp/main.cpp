#include <curl/curl.h>
#include <cstddef>
#include <curl/easy.h>
#include <iostream>
#include <string>
#include <sstream>

const std::string URL = "";
const std::string USERNAME = "";
const std::string PASSWORD = "";
const std::string FROM = "";
const std::string TO = "";

size_t read_callback(char* buffer, size_t size, size_t nitems, void* userdata){
    std::stringstream* ss = static_cast<std::stringstream*>(userdata);
    ss->read(buffer, size * nitems);
    return ss->gcount();
}

int main(){
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(res != CURLE_OK){
        std::cerr << "全局初始化失败: " << curl_easy_strerror(res) << std::endl;
        return -1;
    }

    CURL* curl = curl_easy_init();
    if(!curl){
        std::cerr << "创建 CURL 句柄失败" << std::endl;
        curl_global_cleanup();
        return -1;
    }

    std::stringstream payload;
    payload << "Subject: 测试纯裸写 libcurl\r\n"
            << "Content-Type: text/html; charset=UTF-8\r\n"
            << "\r\n"
            << "<h1>你好！</h1><p>这是没加任何封装的原始邮件发送代码。</p>\r\n";

    struct curl_slist* rcpt_list = nullptr;
    rcpt_list = curl_slist_append(rcpt_list, TO.c_str());

    // 设置URL
    res = curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
    if(res != CURLE_OK){
        std::cerr << "设置URL失败" << std::endl;
        goto cleanup;
    }
    //设置SUERNAME
    res = curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME.c_str());
    if(res != CURLE_OK){
        std::cerr << "设置USERNAME失败" << std::endl;
        goto cleanup;
    }
    // 设置PASSWORD
    res = curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD.c_str());
    if(res != CURLE_OK){
        std::cerr << "设置PASSWORD失败" << std::endl;
        goto cleanup;
    }
    // 设置发件人
    res = curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM.c_str());
    if(res != CURLE_OK){
        std::cerr << "设置发件人失败" << std::endl;
        goto cleanup;
    }
    // 设置收件人
    res = curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, rcpt_list);
    if(res != CURLE_OK){
        std::cerr << "设置发件人列表失败" << std::endl;
        goto cleanup;
    }

    //设置超时
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    // 安全设置：验证服务器证书（生产环境必须有）
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    res = curl_easy_setopt(curl, CURLOPT_READDATA, &payload);
    if(res != CURLE_OK){
        std::cerr << "设置READDATA失败" <<  std::endl;
        goto cleanup;
    }
    res = curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    if(res != CURLE_OK){
        std::cerr << "设置READFUNCTION失败" << std::endl;
        goto cleanup;
    }

    // 开启上传模式
    res = curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    if(res != CURLE_OK){
        std::cerr << "设置上传模式失败" << std::endl;
        goto cleanup;
    }

    // 发送消息
    res = curl_easy_perform(curl);
    if(res != CURLE_OK){
        std::cerr << "邮件发送失败：" << curl_easy_strerror(res) << std::endl;
    }else{
        std::cout << "邮件发送成功，邮件已发送到：" << TO << std::endl;
    }

cleanup:
    if(rcpt_list){
        curl_slist_free_all(rcpt_list);
    }
    if(curl){
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return res == CURLE_OK? 0 : -1;
}
