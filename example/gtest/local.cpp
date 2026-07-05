#include <gtest/gtest.h>
#include <iostream>
#include <unordered_map>

class localTestEnvironment: public testing::Test{
public: 
    static void SetUpTestCase(){
        std::cout << "在所有测试案例进行前执行" << std::endl;
    }
    static void TearDownTestCase(){
        std::cout << "在所有测试案例执行后执行" << std::endl;
    }
    virtual void SetUp() override{
        std::cout << "在每个测试案例进行前执行" << std::endl;
        map.insert({1, 1});
        map.insert({2, 2});
        map.insert({3, 3});
    }
    virtual void TearDown() override{
        std::cout << "在每个测试案例执行后执行" << std::endl;
        map.clear();
    }
public:
    std::unordered_map<int, int> map;
};

// 个数检查
TEST_F(localTestEnvironment, insert){
    ASSERT_EQ(map.size(), 3);
}

// 1存在检查
TEST_F(localTestEnvironment, find){
    ASSERT_NE(map.find(1), map.end());
}

int main(int argc, char* argv[]){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}