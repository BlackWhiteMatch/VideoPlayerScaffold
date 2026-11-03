#include <iostream>
#include <vector>
#include <string>
#include "../../include/VideoPlayerScaffold/utils/ESUtil.h"

// 打印分割线辅助函数
void PrintDivider(const std::string& title) {
    std::cout << "\n========================================================" << std::endl;
    std::cout << ">>> 测试: " << title << std::endl;
    std::cout << "========================================================" << std::endl;
}

// ------------------------------------------------------------
// 1. 测试索引建表构建 (DDL)
// ------------------------------------------------------------
void TestIndexBuilder() {
    PrintDivider("1. 索引建表 (含分词器、权重、禁用索引字段)");

    ESUtil::ESIndexBuilder indexBuilder;
    std::string result = indexBuilder
        .AddCustomAnalyzer("ikmax", "ik_max_word")          // 修正拼写
        .SetDynamic(false)                                  // 严格模式，禁止动态字段
        .AddField("user_id", "keyword")                     // 用户ID精确值
        .AddField("nickname", "text", "ikmax", 2.0)         // 昵称全文检索，权重加倍 2.0
        .AddField("phone", "keyword")                       // 手机号精确值
        .AddField("avatar_url", "keyword", "", 1.0, false)  // 头像只存储，不建立倒排索引
        .AddField("age", "integer")                         // 年龄数值
        .Build();

    std::cout << result << std::endl;
}

// ------------------------------------------------------------
// 2. 测试全量查询 (MatchAll)
// ------------------------------------------------------------
void TestMatchAll() {
    PrintDivider("2. 全量查询 (MatchAll)");

    std::string result = ESUtil::ESQueryBuilder()
        .MatchAll()
        .Build();

    std::cout << result << std::endl;
}

// ------------------------------------------------------------
// 3. 测试标准业务组合查询 (Must + Filter + Range)
// ------------------------------------------------------------
void TestStandardQuery() {
    PrintDivider("3. 核心业务查询 (MustMatch + FilterTerm + FilterRange)");

    std::string result = ESUtil::ESQueryBuilder()
        .MustMatch("nickname", "张三")        // 必须匹配昵称并打分
        .FilterTerm("user_id", "10086")       // 精确过滤用户ID (不打分)
        .FilterRange("age", 18, 35)           // 年龄闭区间: 18 <= age <= 35
        .Build();

    std::cout << result << std::endl;
}

// ------------------------------------------------------------
// 4. 测试多字段检索与多值 Terms 过滤 (MultiMatch + Terms + 单边 Range)
// ------------------------------------------------------------
void TestMultiMatchAndTerms() {
    PrintDivider("4. 高级过滤 (MustMultiMatch + FilterTerms + FilterRangeGte)");

    std::vector<std::string> search_fields = {"nickname", "phone", "description"};
    std::vector<std::string> target_phones = {"15511111111", "15522222222", "15533333333"};

    std::string result = ESUtil::ESQueryBuilder()
        .MustMultiMatch(search_fields, "工程师") // 跨三个字段同时搜 "工程师"
        .FilterTerms("phone", target_phones)     // 手机号必须属于列表之一 (IN 操作)
        .FilterRangeGte("age", 25)               // 单边下限: age >= 25
        .Build();

    std::cout << result << std::endl;
}

// ------------------------------------------------------------
// 5. 测试 Should 模糊加分与阈值控制 (Should + MinimumShouldMatch)
// ------------------------------------------------------------
void TestShouldQuery() {
    PrintDivider("5. 命中加分 (ShouldMatch + ShouldTerms + SetMiniShouldMatch)");

    std::vector<std::string> vip_users = {"VIP_001", "VIP_002"};

    std::string result = ESUtil::ESQueryBuilder()
        .ShouldMatch("nickname", "李四")
        .ShouldTerms("user_id", vip_users)
        .ShouldRangeLte("age", 60)               // 单边上限: age <= 60
        .SetMiniShouldMatch(1)                   // 强制要求 should 列表里至少命中 1 个
        .Build();

    std::cout << result << std::endl;
}

// ------------------------------------------------------------
// 6. 测试完整分页与多字段多级排序 (Sort + From + Size)
// ------------------------------------------------------------
void TestPaginationAndSort() {
    PrintDivider("6. 完整检索 (带分页与复合排序)");

    std::string result = ESUtil::ESQueryBuilder()
        .MustMatch("nickname", "张")
        .Sort("age", true)                       // 第一优先级: 年龄升序 (asc)
        .Sort("created_at", false)               // 第二优先级: 创建时间降序 (desc)
        .SetFrom(10)                             // 从第 10 条开始 (第 2 页)
        .SetSize(10)                             // 每页 10 条
        .Build();

    std::cout << result << std::endl;
}

// ------------------------------------------------------------
// 主函数统一运行
// ------------------------------------------------------------
int main() {
    TestIndexBuilder();
    TestMatchAll();
    TestStandardQuery();
    TestMultiMatchAndTerms();
    TestShouldQuery();
    TestPaginationAndSort();

    std::cout << "\n>>> 所有用例测试完成！\n" << std::endl;
    return 0;
}
