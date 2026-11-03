#include "../../include/VideoPlayerScaffold/utils/ESUtil.h"
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>

namespace ESUtil{
    // ESIndexBuilder Class
    ESIndexBuilder& ESIndexBuilder::AddCustomAnalyzer(const std::string& analyzer_name, const std::string& tokenizer_name){
        Json::Value analyzer_config;
        analyzer_config["type"] = "custom";
        analyzer_config["tokenizer"] = tokenizer_name;

        _root["settings"]["analysis"]["analyzer"][analyzer_name] = analyzer_config;

        return *this;
    }

    ESIndexBuilder& ESIndexBuilder::SetDynamic(bool isDynamic){
        _root["mappings"]["dynamic"] = isDynamic;
        return *this;
    }

    ESIndexBuilder& ESIndexBuilder::SetDynamic(const std::string& mode){
         _root["mappings"]["dynamic"] = mode;
         return *this;
    }

    ESIndexBuilder& ESIndexBuilder::AddField(
        const std::string& field_name,
        const std::string& type,
        const std::string& analyzer,
        double boost,
        bool is_index
    ){
        Json::Value field_config;
        field_config["type"] = type;
        if(!analyzer.empty()){
            field_config["analyzer"] = analyzer;
        }
        if(boost > 1.0){
            field_config["boost"] = boost;
        }
        if(!is_index){
            field_config["index"] = is_index;
        }

        _root["mappings"]["properties"][field_name] = field_config;

        return *this;
    }

    std::string ESIndexBuilder::Build(int indent) const{
        Json::StreamWriterBuilder writer;
        writer["indentation"] = std::string(indent, ' ');
        return Json::writeString(writer, _root);
    }

    // Query class
    ESQueryBuilder& ESQueryBuilder::MatchAll(){
        _is_match_all = true;
        return *this;
    }

    ESQueryBuilder& ESQueryBuilder::SetMiniShouldMatch(int count){
        _mini_should_match = count;
        return *this;
    }

    ESQueryBuilder& ESQueryBuilder::SetFrom(const int from){
        if(from > 0){
            _root["from"] = from;
        }
        return *this;
    }

    ESQueryBuilder& ESQueryBuilder::SetSize(const int size){
        if(size > 0){
            _root["size"] = size;
        }
        return *this;
    }

    ESQueryBuilder& ESQueryBuilder::Sort(const std::string& field, bool asc){
        Json::Value item;
        item[field] = asc ? "asc" : "desc";

        _sort.append(item);
        return *this;
    }

    ESQueryBuilder& ESQueryBuilder::MustMatch(const std::string& field, const std::string& text){
        Json::Value matchItem;
        matchItem["match"][field] = text;
        _must.append(matchItem);
        return *this;
    }

    std::string ESQueryBuilder::Build(int indent) const{
        Json::Value result = _root;
        if(_is_match_all){
            result["query"]["match_all"] = Json::Value(Json::objectValue);
        }else{
            Json::Value bool_query;
            if(!_must.empty()){
                bool_query["must"] = _must;
            }
            if(!_filter.empty()){
                bool_query["filter"] = _filter;
            }
            if(!_should.empty()){
                bool_query["should"] = _should;
            }
            if(_mini_should_match > 0){
                bool_query["minimum_should_match"] = _mini_should_match;
            }

            if(!bool_query.empty()){
                result["query"]["bool"] = bool_query;
            }

            if(!_sort.empty()){
                result["sort"] = _sort;
            }
        }

        Json::StreamWriterBuilder writer;
        writer["indentation"] = std::string(indent, ' ');
        return Json::writeString(writer, result);
    }
}
