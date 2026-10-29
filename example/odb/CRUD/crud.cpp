#include <algorithm>
#include <exception>
#include <iostream>
#include <iterator>
#include <memory>
#include <odb/exception.hxx>
#include <odb/query.hxx>
#include <odb/result.hxx>
#include <vector>

#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/mysql/database.hxx>

#include "../student_table/student.hxx"
#include "../student_table/student-odb.hxx"

void dbC(const std::shared_ptr<odb::mysql::database>& db){
    // 增加操作
    unsigned long id = 2400320225;
    {
        odb::transaction t(db->begin());
        Address addr{ "广西", "桂林", "金鸡路1号"};
        Student stu(id, "张三", addr, 620.5);

        db->persist(stu);
        t.commit();

        std::cout << "成功插入学生：" << stu.name() << ", 学号：" << stu.student_no() << "。" <<std::endl;
    }
}

void dbR(const std::shared_ptr<odb::mysql::database>& db){
    unsigned long id = 2400320225;
    //查找
    {// 主键查询
        odb::transaction t(db->begin());
        std::unique_ptr<Student> stu(db->load<Student>(id));
        std::cout << "查询号学生: " << stu->name() << ", 高考分数: " << stu->entrace_score() << std::endl;

        t.commit();
    }
    {// 条件查询 高考成绩大于600
        odb::transaction t(db->begin());

        typedef odb::query<Student> Query;
        typedef odb::result<Student> Result;

        Result r(db->query<Student>(Query::entrace_score >= 600));
        std::cout << "成绩 >= 600 分的学生名单:" << std::endl;
        for (auto it = r.begin(); it != r.end(); ++it) {
            std::cout << "学号: " << it->student_no()
                        << ", 姓名: " << it->name() << std::endl;
        }

        t.commit();
    }
}

void dbU(const std::shared_ptr<odb::mysql::database>& db){
    odb::transaction t(db->begin());
    std::unique_ptr<Student> stu(db->load<Student>(2400320225));
    Address new_addr{"广西", "玉林", "南江街道"};
    *stu = Student(2400320225, "张三（更新）", new_addr, 650.0);
    db->update(*stu);
    t.commit();
    std::cout << "成功更新学号 " << 2400320225 << " 的信息为: " << stu->name() << std::endl;
}

void dbD(const std::shared_ptr<odb::mysql::database>& db){
    odb::transaction t(db->begin());

    // 按主键删除
    db->erase<Student>(2400320225);
    t.commit();

    std::cout << "成功删除学号为 " << 2400320225 << " 的学生记录。" << std::endl;
}

void executeAction(const std::shared_ptr<odb::mysql::database>& db, std::string option){
    try{
        if(option == "C"){
            dbC(db);
        }else if(option == "R"){
            dbR(db);
        }else if(option == "U"){
            dbU(db);
        }else if(option == "D"){
            dbD(db);
        }
    }catch (const odb::exception& e) {
            std::cerr << "[-] 执行 [" << option << "] 失败 (ODB 错误): " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[-] 执行 [" << option << "] 失败: " << e.what() << std::endl;
        }
}

int main(int argc, char* argv[]){
    if(argc < 2){
        std::cout << "usage: main []" << std::endl;
        return -1;
    }

    try{
        // 连接 mysql数据库
        auto db = std::make_shared<odb::mysql::database>(
            "root",
            "123456",
            "test_db",
            "192.168.204.128",
            3306,
            nullptr,
            "utf8mb4"
        );

        for(size_t i = 0; i < argc; i++){
            std::string option = std::string(argv[i]);
            executeAction(db, option);
        }

    }catch(const odb::exception& e){
        std::cout << "odb error: " << e.what() << std::endl;
        return -1;
    }
    catch(const std::exception& e){
        std::cerr << "sys error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
