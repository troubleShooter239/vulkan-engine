include .env

CXX = g++

CXXFLAGS = -std=c++23 -O2 -Wall -Wextra -g -I.

# Source and output files
SRC_DIR = src
BUILD_DIR = build
BIN = main

INCLUDES = -I"$(VULKAN_INCLUDE_PATH)" -I"$(GLFW_INCLUDE_PATH)" -I"$(GLM_INCLUDE_PATH)"
# LIBS = -L"$(VULKAN_LIB_PATH)" -lvulkan -L"$(GLFW_LIB_PATH)" -lglfw -L"$(GLF_LIB_PATH)" -lglm
# For windows systems
LIBS = -L"$(VULKAN_LIB_PATH)" -lvulkan-1 -L"$(GLFW_LIB_PATH)" -lglfw3 -L"$(GLF_LIB_PATH)" -lglm -lgdi32

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Rules
.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDES) $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(BIN)
