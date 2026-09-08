# ============================================================================
# 1. 专属变量配置区
# ============================================================================
# [1] 编译产物存放的细分目录 (建议指向根目录的 bin)
OUT_DIR = ../../bin/example-utils-EtcdUtil-demo
# [2] 最终生成的可执行文件完整路径 —— 必须是文件名，不能是目录！
TARGET  = $(OUT_DIR)/Service-demo
TARGET1 = $(OUT_DIR)/Client-demo
# [3] 当前需要编译的所有源码文件 (如果有多个，用空格隔开)
SRCS  = Service.cpp ../../src/utils/EtcdUtil.cpp ../../src/utils/Logger.cpp
SRCS1 = Client.cpp  ../../src/utils/EtcdUtil.cpp ../../src/utils/Logger.cpp
# [4] 编译选项：-I 放到 CXXFLAGS，-lxxx 放到 LIBS，分开更规范
CXXFLAGS = -I ../../include/VideoPlayerScaffold/utils/
LIBS     = -lspdlog -lgflags -lfmt -lpthread -letcd-cpp-api -lcpprest
# ============================================================================
# 2. 核心编译引擎区
# ============================================================================
# 声明伪目标，防止和当前目录下偶然创建的同名普通文件发生冲突
.PHONY: all clean run
# 默认敲 make 时只执行编译
all: $(TARGET) $(TARGET1)

# order‑only | $(OUT_DIR)：保证目录先创建，不参与时间戳比较
$(TARGET): $(SRCS) | $(OUT_DIR)
	g++ -o $@ $^ $(CXXFLAGS) $(LIBS)

$(TARGET1): $(SRCS1) | $(OUT_DIR)
	g++ -o $@ $^ $(CXXFLAGS) $(LIBS)

# 创建输出目录规则
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# 一键大扫除规则
clean:
	@echo "🧹 正在为你无情清扫编译垃圾..."
	rm -rf $(OUT_DIR)
	@echo "✨ 清理完毕！源码树已恢复绝对纯洁！"
