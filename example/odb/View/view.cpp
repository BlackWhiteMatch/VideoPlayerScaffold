#include <exception>
#include <iostream>
#include <memory>
#include <odb/database.hxx>
#include <odb/exception.hxx>
#include <odb/query.hxx>
#include <odb/transaction.hxx>
#include <odb/mysql/database.hxx>

#include "../student_table/student.hxx"
#include "../student_table/student-odb.hxx"

void perpareData(const std::shared_ptr<odb::mysql::database>& db);
void queryStudentWithClass(const std::shared_ptr<odb::mysql::database>& db);

int main(){
    try{
        auto db = std::make_shared<odb::mysql::database>(
            "root",
            "123456",
            "test_db",
            "192.168.204.128",
            3306,
            nullptr,
            "utf8mb4"
        );

        // 准备数据
        perpareData(db);

        // 测试查询
        queryStudentWithClass(db);

    }catch(const odb::exception& e){
        std::cerr << "ODB Error: " << e.what() << std::endl;
        return -1;
    }catch(const std::exception& e){
        std::cerr << "Other Error: " << e.what() << std::endl;
        return -1;
    }
}


void perpareData(const std::shared_ptr<odb::mysql::database>& db){
    odb::transaction t(db->begin());

    Classes c1(1, "计算机科学与技术1班");
    Classes c2(2, "软件工程1班");
    db->persist(c1);
    db->persist(c2);

    Address addr1{"广西", "桂林", "金鸡路1号"};
    Address addr2{"北京", "海淀", "中关村大街"};

    Student s1(2400320225, 1, "张三", addr1, 620.5);
    Student s2(2400320226, 1, "李四", addr1, 595.0);
    Student s3(2400320227, 2, "王五", addr2, 640.0);

    db->persist(s1);
    db->persist(s2);
    db->persist(s3);

    t.commit();
    std::cout << "测试数据插入成功！" << std::endl;
}

void queryStudentWithClass(const std::shared_ptr<odb::mysql::database>& db){
    odb::transaction t(db->begin());

    typedef odb::query<StudentClassView> ViewQuery;
    typedef odb::result<StudentClassView> ViewResult;

    ViewResult r(db->query<StudentClassView>(
        ViewQuery::Classes::name == "计算机科学与技术1班" &&
        ViewQuery::Student::entrace_score >= 600.0
    ));

    std::cout << "\n--- 连表查询结果 (计科1班 & 分数 >= 600) ---" << std::endl;
    for (auto it = r.begin(); it != r.end(); ++it) {
        std::cout << "学号: " << it->student_no
        << ", 姓名: " << it->student_name
        << ", 班级: " << it->class_name
        << ", 分数: " << it->score << std::endl;
    }

    t.commit();
}
