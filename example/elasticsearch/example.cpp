#include <chrono>
#include <cstdio>
#include <iostream>
#include <cpr/response.h>
#include <elasticlient/client.h>
#include <thread>

const std::string index_name{"student"};
const std::string type_name{"_doc"};
const std::string index_id{"default_id"};

void create_index(elasticlient::Client& client){
    const std::string body{R"({
      "settings": {
        "analysis": {
          "analyzer": {
            "ikmax": {
              "type": "custom",
              "tokenizer": "ik_max_word"
            }
          }
        }
      },
      "mappings": {
        "dynamic": false,
        "properties": {
          "name": {
            "type": "text",
            "boost": 3.0,
            "analyzer": "ikmax"
          },
          "age": {
            "type": "integer"
          },
          "phone": {
            "type": "keyword",
            "boost": 1.0
          },
          "skills": {
            "type": "text"
          },
          "birth": {
            "type": "date",
            "index": false
          }
        }
      }
    })"};

    auto response = client.index(index_name, type_name, index_id, body);
    if(response.status_code < 200 || response.status_code >= 300){
        std::cout << "create index failed: " << response.status_code;
        std::cout << " " << response.text << std::endl;
        return;
    }else{
        std::cout << "Initliazed client successfully!" << std::endl;
        return;
    }
}

void add_data(elasticlient::Client& client){
    const std::string body{
        R"({
            "name": "张三",
            "age": 19,
            "phone": "13888888888",
            "skills": ["java","php","go"],
            "birth": "2007-05-21 12:35:32"
        })"
    };

    auto response = client.index(index_name, type_name, "3", body);
    if(response.status_code < 200 || response.status_code >= 300){
        std::cout << "add_data failed: " << response.status_code;
        std::cout << " " << response.text << std::endl;
        return;
    }else{
        std::cout << "Add data successfully!" << std::endl;
        return;
    }
}

void mod_data(elasticlient::Client& client){
    const std::string body{
        R"({
            "name": "李四",
            "age": 20,
            "phone": "13888888888",
            "skills": ["java","php","C++"],
            "birth": "2007-05-21 12:35:32"
        })"
    };

    auto response = client.index(index_name, type_name, "2", body);
    if(response.status_code < 200 || response.status_code >= 300){
        std::cout << "mod_data failed: " << response.status_code;
        std::cout << " " << response.text << std::endl;
        return;
    }else{
        std::cout << "Mod data successfully!" << std::endl;
        return;
    }
}

void remove_data(elasticlient::Client& client){
    auto response = client.remove(index_name, type_name, "2");

    if(response.status_code < 200 || response.status_code >= 300){
        std::cout << "remove_data failed: " << response.status_code;
        std::cout << " " << response.text << std::endl;
        return;
    }else{
        std::cout << "Remove data successfully!" << std::endl;
        return;
    }
}

void remove_index(elasticlient::Client& client){
    auto response = client.performRequest(
        elasticlient::Client::HTTPMethod::DELETE,
        index_name,
        ""
    );

    if(response.status_code < 200 || response.status_code >= 300){
        std::cout << "remove_index failed: " << response.status_code;
        std::cout << " " << response.text << std::endl;
        return;
    }else{
        std::cout << "Remove index successfully!" << std::endl;
        return;
    }
}

void get_index(elasticlient::Client& client){
    std::string body{
        R"(
            {
                "query" :  {
                    "match_all" :   {}
                }
            }
        )"
    };

    auto response = client.search(index_name, type_name, body);

    if(response.status_code < 200 || response.status_code >= 300){
        std::cout << "get_index failed: " << response.status_code;
        std::cout << " " << response.text << std::endl;
        return;
    }else{
        std::cout << "Get index successfully!" << std::endl;
        return;
    }
}

int main(){
    const std::string url("http://elastic:123456@192.168.204.128:9200/");
    // 创建ElasticSearch
    elasticlient::Client client({url});

    //创建index
    // create_index(client);
    // std::this_thread::sleep_for(std::chrono::seconds(1));

    //插入数据
    // add_data(client);
    // std::this_thread::sleep_for(std::chrono::seconds(1));

    // 修改数据
    // mod_data(client);
    // std::this_thread::sleep_for(std::chrono::seconds(1));

    //删除数据
    // remove_data(client);
    // std::this_thread::sleep_for(std::chrono::seconds(1));

    //删除索引
    // remove_index(client);
    // std::this_thread::sleep_for(std::chrono::seconds(1));

    //查询index
    // get_index(client);
    // std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
