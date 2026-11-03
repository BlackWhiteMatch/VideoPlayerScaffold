#include <iostream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>


namespace ESUtil {

    class ESIndexBuilder{
    public:
        ESIndexBuilder() :_root(Json::objectValue){}

        // 添加自定义分词器
        ESIndexBuilder& AddCustomAnalyzer(const std::string& analyzer_name, const std::string& tokenizer_name);

        //设置dynamic
        ESIndexBuilder& SetDynamic(bool isDynamic);

        ESIndexBuilder& SetDynamic(const std::string& mode);

        //添加字段映射
        ESIndexBuilder& AddField(
            const std::string& field_name,
            const std::string& type,
            const std::string& analyzer = "",
            double boost = 1.0,
            bool is_index = true
        );

        // 构建JSON字符串
        std::string Build(int indent = 2) const;

    private:
        Json::Value _root;
    };


    // Query Class
    class ESQueryBuilder{
    public:
        ESQueryBuilder() :_root(Json::objectValue){}

        ESQueryBuilder& MatchAll();
        ESQueryBuilder& SetMiniShouldMatch(int count = 0);
        ESQueryBuilder& SetFrom(const int from = 0);
        ESQueryBuilder& SetSize(const int size = 0);
        ESQueryBuilder& Sort(const std::string& field, bool asc = false);

        ESQueryBuilder& MustMatch(const std::string& field, const std::string& text);
        template<typename Container>
        ESQueryBuilder& MustMultiMatch(const Container& fields, const std::string& text){
            Json::Value arr(Json::arrayValue);
            for(auto& e : fields){
                arr.append(e);
            }
            Json::Value item;
            item["multi_match"]["query"] = text;
            item["multi_match"]["fields"] = arr;
            _must.append(item);
            return *this;
        }
        template<typename T>
        ESQueryBuilder& MustTerm(const std::string& field, const T& value){
            Json::Value termItem;
            termItem["term"][field] = value;
            _must.append(termItem);
            return *this;
        }
        template<typename T>
        ESQueryBuilder& FilterTerm(const std::string& field, const T& value){
            Json::Value termItem;
            termItem["term"][field] = value;
            _filter.append(termItem);
            return *this;
        }
        template<typename Container>
        ESQueryBuilder& FilterTerms(const std::string& field, const Container& values){
            Json::Value arr(Json::arrayValue);
            for(auto& e : values){
                arr.append(e);
            }
            Json::Value item;
            item["terms"][field] = arr;
            _filter.append(item);
            return *this;
        }
        template<typename T>
        ESQueryBuilder& FilterRange(const std::string& field, const T& gte, const T& lte){
            Json::Value rangeOptions;
            rangeOptions["gte"] = gte;
            rangeOptions["lte"] = lte;

            Json::Value rangeItem;
            rangeItem["range"][field] = rangeOptions;
            _filter.append(rangeItem);
            return *this;
        }
        template<typename T>
        ESQueryBuilder& FilterRangeGte(const std::string& field, const T& gte){
            Json::Value Option;
            Option["gte"] = gte;

            Json::Value gteItem;
            gteItem["range"][field] = Option;
            _filter.append(gteItem);
            return *this;
        }
        template<typename T>
        ESQueryBuilder& FilterRangeLte(const std::string& field, const T& lte){
            Json::Value Option;
            Option["lte"] = lte;

            Json::Value lteItem;
            lteItem["range"][field] = Option;
            _filter.append(lteItem);
            return *this;
        }

        ESQueryBuilder& ShouldMatch(const std::string& field, const std::string& text){
            Json::Value item;
            item["match"][field] = text;
            _should.append(item);
            return *this;
        }
        template<typename T>
        ESQueryBuilder& ShouldTerm(const std::string& field, const T& value){
            Json::Value item;
            item["term"][field] = value;
            _should.append(item);
            return *this;
        }
        template<typename T>
        ESQueryBuilder& ShouldTerms(const std::string& field, const T& values){
            Json::Value arr(Json::arrayValue);
            for(auto& e : values){
                arr.append(e);
            }

            Json::Value item;
            item["terms"][field] = arr;
            _should.append(item);
            return *this;
        }
        template<typename T>
        ESQueryBuilder& ShouldRange(const std::string& field, const T& gte, const T& lte){
            Json::Value rangeOptions;
            rangeOptions["gte"] = gte;
            rangeOptions["lte"] = lte;

            Json::Value rangeItem;
            rangeItem["range"][field] = rangeOptions;
            _should.append(rangeItem);
            return *this;
        }
        template<typename T>
        ESQueryBuilder& ShouldRangeGte(const std::string& field, const T& gte){
            Json::Value Option;
            Option["gte"] = gte;

            Json::Value gteItem;
            gteItem["range"][field] = Option;
            _should.append(gteItem);
            return *this;
        }
        template<typename T>
        ESQueryBuilder& ShouldRangeLte(const std::string& field, const T& lte){
            Json::Value Option;
            Option["lte"] = lte;

            Json::Value lteItem;
            lteItem["range"][field] = Option;
            _should.append(lteItem);
            return *this;
        }

        std::string Build(int indent = 2) const;

        operator std::string(){
            return Build();
        }

    private:
        bool _is_match_all{false};
        int _mini_should_match{ 0 };
        Json::Value _root;
        Json::Value _must{ Json::arrayValue };
        Json::Value _filter{ Json::arrayValue };
        Json::Value _should { Json::arrayValue };
        Json::Value _sort { Json::arrayValue };
    };

}
