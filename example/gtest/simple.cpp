#include <gtest/gtest.h>
#include <iostream>

TEST(unorder_map, insert){
    std::unordered_map<int, int> map;
    map.insert({1, 1});
    map.insert({2, 2});
    map.insert({3, 3});
    ASSERT_EQ(map.size(), 3);
}

TEST(unorder_map, find){
    std::unordered_map<int, int> map;
    map.insert({1, 1});
    map.insert({2, 2});
    map.insert({3, 3});
    ASSERT_EQ(map.find(1)->second, 1);
    ASSERT_NE(map.find(1), map.end());
}
// 无法通过的测试
TEST(unorder_map, find_can_not_pass){
    std::unordered_map<int, int> map;
    map.insert({1, 1});
    map.insert({2, 2});
    map.insert({3, 3});
    ASSERT_EQ(map.find(1)->second, 1);
    ASSERT_NE(map.find(4), map.end());
}

int main(int argc, char* argv[]){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}