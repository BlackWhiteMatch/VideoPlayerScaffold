#include "../../include/VideoPlayerScaffold/utils/MailCodeUtil.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <memory.h>
#include <memory>

namespace MailCodeUtil{
    SMTPClient::SMTPClient(const MailCofig& config) : _config(config){
        curl_global_init(CURL_GLOBAL_DEFAULT);
    };
    SMTPClient::~SMTPClient(){
        curl_global_cleanup();
    }

    bool SMTPClient::send(const MailMessage& message){
        CurlHandle curl(curl_easy_init());
        if(!curl){
            return false;
        }

        curl_easy_setopt(curl.get(), CURLOPT_URL, _config.url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_USERNAME, _config.username.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_PASSWORD, _config.password.c_str());

        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 30L);

        curl_easy_setopt(curl.get(), CURLOPT_MAIL_FROM, _config.from_addr.c_str());

        curl_slist* raw_rcpt = nullptr;
        for (const auto& to : message.to_addrs) {
            raw_rcpt = curl_slist_append(raw_rcpt, to.c_str());
        }
        CurlSlist recipients(raw_rcpt);
        curl_easy_setopt(curl.get(), CURLOPT_MAIL_RCPT, recipients.get());

         std::string raw_payload = buildPayload(message);

         curl_easy_setopt(curl.get(), CURLOPT_UPLOAD, 1L);

         auto ctx = std::make_shared<UploadContext>(std::move(raw_payload));
         curl_easy_setopt(curl.get(), CURLOPT_READDATA, ctx.get());
         curl_easy_setopt(curl.get(), CURLOPT_READFUNCTION, &readCallback);

         // 发送邮件
         CURLcode res = curl_easy_perform(curl.get());
         if(res != CURLE_OK){
             std::cerr << "发送邮件失败: " << curl_easy_strerror(res) << std::endl;
             return false;
         }
         return true;
    }

    std::string SMTPClient::buildPayload(const MailMessage& message) const{
        std::string payload;
        payload += "Subject: " + message.subject + "\r\n";
        payload += "Content-Type: text/" + std::string(message.is_html ? "html" : "plain") + "; charset=UTF-8\r\n";
        payload += "\r\n";
        payload += message.body + "\r\n";
        return payload;
    }

    size_t SMTPClient::readCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
        auto* ctx_ptr = static_cast<UploadContext*>(userdata);
        if (ctx_ptr->offest >= ctx_ptr->payload.length()) {
            return 0;
        }

        size_t max_size = size * nitems;
        size_t remaining = ctx_ptr->payload.length() - ctx_ptr->offest;
        size_t len_to_copy = std::min(max_size, remaining);

        if (len_to_copy > 0) {
            memcpy(buffer, ctx_ptr->payload.c_str() + ctx_ptr->offest, len_to_copy);
            ctx_ptr->offest += len_to_copy;
            return len_to_copy;
        }
        return 0;
    }
}
