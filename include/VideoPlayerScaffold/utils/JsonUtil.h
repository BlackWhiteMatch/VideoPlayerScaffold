#pragma once
#include <optional>
#include <jsoncpp/json/json.h>

namespace JsonUtil {
    class JSON {
    public:
        static std::optional<std::string> Serialize(const Json::Value& val);
        static std::optional<Json::Value> UnSerialize(const std::string& str);
    };
};