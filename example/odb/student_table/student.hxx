#pragma once
#include <odb/forward.hxx>
#include <string>
#include <odb/core.hxx>
#include <utility>

#pragma db value
struct Address {
    std::string province;
    std::string city;
    std::string street;
};

#pragma db object table("t_student")
class Student {
public:
    Student() = default;
    Student(unsigned long student_no, unsigned long class_id, std::string name, Address addr, double score)
        : _student_no(student_no), _class_id(class_id), _name(std::move(name)), _addr(std::move(addr)), _entrace_score(score), _is_online(false)
    {

    }

    std::string name() { return _name; }
    unsigned long student_no() { return _student_no; }
    double entrace_score() { return _entrace_score; }

private:
    friend class odb::access;
    #pragma db id column("f_student_no")
    unsigned long _student_no = 0;

    #pragma db column("class_id") not_null
    unsigned long _class_id;

    #pragma db type("VARCHAR(32)") column("name") not_null
    std::string _name;

    #pragma db column("addr_")
    Address _addr;

    #pragma db default("0.0") column("entrace_score")
    double _entrace_score = 0.0;

    #pragma db transient
    bool _is_online = false;
};


// 创建班级表
#pragma db object table("classes")
class Classes {
public:
    Classes() = default;
    Classes(unsigned long id, const std::string& name)
        :_id(id), _name(name) {}

    unsigned long id() const { return _id; }
    const std::string& name() const { return _name; }
    void name(const std::string& new_name) { _name = new_name; }
private:
    friend class odb::access;
    #pragma db id auto
    unsigned long _id;

    #pragma db type("VARCHAR(64)") not_null
    std::string _name;
};

#pragma db view object(Student) \
                object(Classes: Student::_class_id == Classes::_id)
struct StudentClassView{
    #pragma db column(Student::_student_no)
    unsigned long student_no;

    #pragma db column(Student::_name)
    std::string student_name;

    #pragma db column(Classes::_name)
    std::string class_name;

    #pragma db column(Student::_entrace_score)
    double score;
};
