# ============================================================================
# 1. 专属变量配置区
# ============================================================================

# [1] 编译产物存放的细分目录 (建议指向根目录的 bin)
OUT_DIR = ../../bin/example-jsoncpp-demo

# [2] 最终生成的可执行文件完整路径
TARGET_t = $(OUT_DIR)/test
TARGET_t1 = $(OUT_DIR)/test1
TARGET_t2 = $(OUT_DIR)/test2

# [3] 当前需要编译的所有源码文件 (如果有多个，用空格隔开)
SRCS_t = test.cpp
SRCS_t1 = test1.cpp
SRCS_t2 = test2.cpp

# [4] 编译时需要链接的外部动态库 (如果没有就留空，比如 -ljsoncpp)
LIBS = -ljsoncpp

# ============================================================================
# 2. 核心编译引擎区
# ============================================================================

# 声明伪目标，防止和当前目录下偶然创建的同名普通文件发生冲突
.PHONY: all clean run_t run_t1 run_t2

# 默认敲 make 时只执行编译
all: $(TARGET_t) $(TARGET_t1) $(TARGET_t2)

# 真正的编译规则：自动建目录，自动链接，自动输出
$(TARGET_t): $(SRCS_t)
	mkdir -p $(OUT_DIR)
	g++ -o $@ $^ $(LIBS)
$(TARGET_t1): $(SRCS_t1)
	mkdir -p $(OUT_DIR)
	g++ -o $@ $^ $(LIBS)
$(TARGET_t2): $(SRCS_t2)
	mkdir -p $(OUT_DIR)
	g++ -o $@ $^ $(LIBS)

# 一键运行规则
run_t: $(TARGET_t)
	@$(TARGET_t)
run_t1: $(TARGET_t1)
	@$(TARGET_t1)
run_t2: $(TARGET_t2)
	@$(TARGET_t2)

# 一键大扫除规则
clean:
	@echo "🧹 正在为你无情清扫编译垃圾..."
	rm -rf $(OUT_DIR)
	@echo "✨ 清理完毕！源码树已恢复绝对纯洁！"