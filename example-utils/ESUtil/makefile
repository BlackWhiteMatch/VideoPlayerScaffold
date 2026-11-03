# ============================================================================
# 1. 专属变量配置区
# ============================================================================
OUT_DIR = ../../bin/example-utils-ESUtil-demo

TARGET  = $(OUT_DIR)/demo


SRCS  = main.cpp ../../src/utils/ESUtil.cpp

CXXFLAGS = -std=c++17 -I ../../include/
LIBS     = -ljsoncpp

# ============================================================================
# 2. 核心编译引擎区
# ============================================================================
.PHONY: all clean run

all: $(TARGET) $(TARGET1)

$(TARGET): $(SRCS) | $(OUT_DIR)
	g++ -o $@ $^ $(CXXFLAGS) $(LIBS)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

clean:
	@echo "🧹 正在为你无情清扫编译垃圾..."
	rm -rf $(OUT_DIR)
	@echo "✨ 清理完毕！源码树已恢复绝对纯洁！"
