#include "../../include/VideoPlayerScaffold/utils/MailCodeUtil.h"
#include <string>
#include <vector>

int main(){
    MailCodeUtil::MailCofig config{
        .url = "smtps://smtp.163.com",
        .username = "",
        .password = "",
        .from_addr = ""
    };

    MailCodeUtil::MailMessage message{
        .to_addrs = std::vector<std::string>{""},
        .subject = "MailCodeUtilTest",
        .body = "<h1>你好！</h1><p>这是没加任何封装的原始邮件发送代码。</p>",
        .is_html = true
    };

    MailCodeUtil::SMTPClient smtpClient(config);

    bool result = smtpClient.send(message);
    if(result){
        for(auto& e : message.to_addrs){
            std::cout << "发送邮件到：" << e << "成功！" << std::endl;
        }
    }
    return 0;
}
