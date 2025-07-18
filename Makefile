# Makefile

# .env 파일이 있으면 포함하고, 변수들을 export 합니다.
# 파일이 없어도 오류를 발생시키지 않습니다.
-include .env
export

CXX        := clang++
CXXFLAGS   := -std=c++17 -Wall \
               -Isrc \
               -ILib/hook/install_event_tap \
               -ILib/hook/event_callback \
               -ILib/hook/replacement \
               -ILib/hook/post_event \
               -ILib/hook \
               -ILib/API
LDFLAGS    := -framework ApplicationServices -lcurl

SRC_DIR    := src
LIB_DIR    := Lib

BUILD_DIR  := Build
OUT_DIR    := Output

TARGET     := keyhook

# main.cpp
SRCS       := $(wildcard $(SRC_DIR)/*.cpp)
# Lib/hook/*/*.cpp 전부 + Lib/API/*.cpp 전부
LIB_SRCS   := $(wildcard $(LIB_DIR)/hook/*/*.cpp) $(wildcard Lib/API/*.cpp)

# 객체 파일 경로
SRCS_OBJ   := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
LIB_OBJ    := $(patsubst $(LIB_DIR)/hook/%.cpp,$(BUILD_DIR)/hook/%.o,$(filter $(LIB_DIR)/hook/%.cpp,$(LIB_SRCS))) \
              $(patsubst Lib/API/%.cpp,$(BUILD_DIR)/API/%.o,$(filter Lib/API/%.cpp,$(LIB_SRCS)))
OBJS       := $(SRCS_OBJ) $(LIB_OBJ)

.PHONY: all clean

all: $(OUT_DIR)/$(TARGET)

# 최종 링크
$(OUT_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(OUT_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# src/*.cpp → Build/*.o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Lib/hook/**/*.cpp → Build/hook/.../*.o
$(BUILD_DIR)/hook/%.o: $(LIB_DIR)/hook/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Lib/API/*.cpp → Build/API/*.o
$(BUILD_DIR)/API/%.o: Lib/API/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(OUT_DIR)