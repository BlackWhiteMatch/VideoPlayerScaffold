#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <curl/curl.h>

namespace MailCodeUtil{
    struct MailCofig{
        std::string url;
        std::string username;
        std::string password;
        std::string from_addr;
    };

    struct MailMessage{
        std::vector<std::string> to_addrs;
        std::string subject;
        std::string body;
        bool is_html = true;
    };

    struct CurlDeleter{
        void operator()(CURL* curl){
            if(!curl){
                curl_easy_cleanup(curl);
            }
        }
    };
    using CurlHandle = std::unique_ptr<CURL, CurlDeleter>;

    struct SlistDeleter{
        void operator()(curl_slist* slist){
            if(!slist){
                curl_slist_free_all(slist);
            }
        }
    };
    using CurlSlist = std::unique_ptr<curl_slist, SlistDeleter>;

    struct UploadContext {
        const std::string payload;
        size_t offest = 0;

        explicit UploadContext(std::string&& pl) :payload(std::move(pl)){}
    };

    class SMTPClient{
    public:
        explicit SMTPClient(const MailCofig& config);
        ~SMTPClient();

        bool send(const MailMessage& message);
    private:
        std::string buildPayload(const MailMessage& message) const;
        static size_t readCallback(char* buffer, size_t size, size_t nitems, void* userdata);

        MailCofig _config;
    };
}
