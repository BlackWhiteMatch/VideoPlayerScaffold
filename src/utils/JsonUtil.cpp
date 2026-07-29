#include "../../include/VideoPlayerScaffold/utils/JsonUtil.h"
#include "../../include/VideoPlayerScaffold/utils/Logger.h"
#include <exception>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <optional>
#include <sstream>

namespace JsonUtil {
    std::optional<std::string> JSON::Serialize(const Json::Value& val){
        Json::StreamWriterBuilder builder;
        builder.settings_["indentation"] = "";
        try{
            std::string result = Json::writeString(builder, val);
            return result;
        }catch(const std::exception& e){
            LOG_ERROR("Serialize failed! Exception: {}", e.what());
            return std::nullopt;
        }
    }
    std::optional<Json::Value> JSON::UnSerialize(const std::string& str){
        Json::CharReaderBuilder builder;
        Json::Value val;
        std::string errStr;
        std::istringstream iss(str);

        bool res = Json::parseFromStream(builder, iss, &val, &errStr);
        if(!res){
            LOG_ERROR("UnSerialize read failed: {}", errStr);
            return std::nullopt;
        }
        return val; 
    }
};