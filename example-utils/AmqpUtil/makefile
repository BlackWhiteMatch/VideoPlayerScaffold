# ============================================================================
# 1. 专属变量配置区
# ============================================================================
OUT_DIR = ../../bin/example-utils-AmqpUtil-demo

TARGET  = $(OUT_DIR)/Producer-demo
TARGET1 = $(OUT_DIR)/Consumer-demo

SRCS  = ProducerDemo.cpp ../../src/utils/AmqpUtil.cpp
SRCS1 = ConsumerDemo.cpp ../../src/utils/AmqpUtil.cpp

CXXFLAGS = -std=c++17 -I ../../include/
LIBS     = -lamqpcpp -lev -lpthread

# ============================================================================
# 2. 核心编译引擎区
# ============================================================================
.PHONY: all clean run

all: $(TARGET) $(TARGET1)

$(TARGET): $(SRCS) | $(OUT_DIR)
	g++ -o $@ $^ $(CXXFLAGS) $(LIBS)

$(TARGET1): $(SRCS1) | $(OUT_DIR)
	g++ -o $@ $^ $(CXXFLAGS) $(LIBS)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

clean:
	@echo "🧹 正在为你无情清扫编译垃圾..."
	rm -rf $(OUT_DIR)
	@echo "✨ 清理完毕！源码树已恢复绝对纯洁！"