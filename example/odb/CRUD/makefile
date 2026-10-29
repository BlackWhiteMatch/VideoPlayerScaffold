# ============================================================================
# 1. 专属变量配置区
# ============================================================================

# [1] 编译产物存放的细分目录 (建议指向根目录的 bin)
OUT_DIR = ../../../bin/example-odb-demo/CRUD

# [2] 最终生成的可执行文件完整路径
TARGET = $(OUT_DIR)/CRUD_test

# [3] 当前需要编译的所有源码文件 (加上 student_table 目录下的 student-odb.cxx)
SRCS = crud.cpp ../student_table/student-odb.cxx

# [4] 头文件包含目录 (让 crud.cpp 能直接找到 student.hxx 等头文件)
INCLUDES = -I../student_table

# [5] 编译时需要链接的外部动态库
LIBS = -lodb-mysql -lodb -lmysqlclient -lpthread -ldl


# ============================================================================
# 2. 核心编译引擎区
# ============================================================================

.PHONY: all clean run

all: $(TARGET)

# 编译规则：加入 $(INCLUDES) 支持头文件搜索
$(TARGET): $(SRCS)
	mkdir -p $(OUT_DIR)
	g++ -std=c++17 $(INCLUDES) -o $@ $^ $(LIBS)

run: $(TARGET)
	@$(TARGET)

clean:
	@echo "🧹 正在为你无情清扫编译垃圾..."
	rm -rf $(OUT_DIR)
	@echo "✨ 清理完毕！源码树已恢复绝对纯洁！"
