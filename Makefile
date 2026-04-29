CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Iinclude
BUILD    := build
TARGET   := DataBackupPlanner

SRCS := src/main.cpp src/File.cpp src/BackupManager.cpp src/RecoverySystem.cpp
OBJS := $(patsubst src/%.cpp, $(BUILD)/%.o, $(SRCS))

.PHONY: all clean run test

all: $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET): $(OBJS)
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "\n  Build successful -> $@\n"

$(BUILD)/%.o: src/%.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(BUILD)/$(TARGET)

test: $(BUILD)/RunTests
	./$(BUILD)/RunTests

$(BUILD)/RunTests: tests/test_main.cpp src/File.cpp src/BackupManager.cpp src/RecoverySystem.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf $(BUILD)
	@echo "  Cleaned build artifacts."
