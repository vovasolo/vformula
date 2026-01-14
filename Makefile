# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I. -O2
VFLAGS := -I/usr/include/eigen3 
LDFLAGS :=

# Directories
SRC_DIR := .
TEST_DIR := tests
VECTEST_DIR := vector_tests

# Source files
SRC := $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRC := $(wildcard $(TEST_DIR)/*.cpp)
VECTEST_SRC := $(wildcard $(VECTEST_DIR)/*.cpp)

# Build directories
BUILD_DIR := build
BIN_DIR := bin

# Targets
TARGET_LIB := $(BUILD_DIR)/vformula.o
TEST_BINS := $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/%,$(TEST_SRC))
VECTEST_BINS := $(patsubst $(VECTEST_DIR)/%.cpp,$(BIN_DIR)/%,$(VECTEST_SRC))

.PHONY: all tests vectests clean

all: tests

# Build object for library
$(TARGET_LIB): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $^ -o $@

# Build each test executable
$(BIN_DIR)/%: $(TEST_DIR)/%.cpp $(TARGET_LIB)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< $(TARGET_LIB) -o $@ $(LDFLAGS)

$(BIN_DIR)/%: $(VECTEST_DIR)/%.cpp $(TARGET_LIB)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(VFLAGS) $< $(TARGET_LIB) -o $@ $(LDFLAGS)

tests: $(TEST_BINS)

vectests: $(VECTEST_BINS)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
