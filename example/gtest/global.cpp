#include <gtest/gtest.h>
#include <iostream>

std::unordered_map<int, int> map;

class GlobalTestEnvironment: public testing::Environment{
public: 
    virtual void SetUp() override{
        std::cout << "在所有测试案例进行前执行" << std::endl;
    }
    virtual void TearDown() override{
        std::cout << "在所有测试案例执行后执行" << std::endl;
    }
};

TEST(unorder_map, insert){
    map.insert({1, 1});
    map.insert({2, 2});
    map.insert({3, 3});
    ASSERT_EQ(map.size(), 3);
}

TEST(unorder_map, find){
    ASSERT_NE(map.find(1), map.end());
}

int main(int argc, char* argv[]){
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new GlobalTestEnvironment());
    return RUN_ALL_TESTS();
}