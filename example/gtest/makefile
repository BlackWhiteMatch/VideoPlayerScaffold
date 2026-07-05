# ============================================================================
# 1. 专属变量配置区
# ============================================================================

# [1] 编译产物存放的细分目录 (建议指向根目录的 bin)
OUT_DIR = ../../bin/example-gtest-demo

# [2] 最终生成的可执行文件完整路径
TARGET_S = $(OUT_DIR)/simple
TARGET_G = $(OUT_DIR)/global
TARGET_L = $(OUT_DIR)/local

# [3] 当前需要编译的所有源码文件 (如果有多个，用空格隔开)
SRCS_S = simple.cpp
SRCS_G = global.cpp
SRCS_L = local.cpp

# [4] 编译时需要链接的外部动态库 (如果没有就留空，比如 -ljsoncpp)
LIBS = -lgtest


# ============================================================================
# 2. 核心编译引擎区
# ============================================================================

# 声明伪目标，防止和当前目录下偶然创建的同名普通文件发生冲突
.PHONY: all clean run_simple run_global run_local

# 默认敲 make 时只执行编译
all: $(TARGET_S) $(TARGET_G) $(TARGET_L)

# 真正的编译规则：自动建目录，自动链接，自动输出
$(TARGET_S): $(SRCS_S)
	mkdir -p $(OUT_DIR)
	g++ -o $@ $^ $(LIBS)
$(TARGET_G): $(SRCS_G)
	mkdir -p $(OUT_DIR)
	g++ -o $@ $^ $(LIBS)
$(TARGET_L): $(SRCS_L)
	mkdir -p $(OUT_DIR)
	g++ -o $@ $^ $(LIBS)

# 一键运行规则
run_global: $(TARGET_G)
	@$(TARGET_G)
run_simple: $(TARGET_S)
	@$(TARGET_S)
run_local: $(TARGET_L)
	@$(TARGET_L)

# 一键大扫除规则
clean:
	@echo "🧹 正在为你无情清扫编译垃圾..."
	rm -rf $(OUT_DIR)
	@echo "✨ 清理完毕！源码树已恢复绝对纯洁！"