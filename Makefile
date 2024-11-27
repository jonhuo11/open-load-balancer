# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++17 -O2

# Directories
SRC_DIR = src
BUILD_DIR = build

# Find all source files with .cc extension
SRCS = $(wildcard $(SRC_DIR)/*.cc)

# Generate corresponding object files in build directory
OBJS = $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRCS))

# Generate dependency files
DEPS = $(OBJS:.o=.d)

# Executable name (use the directory name or a custom name)
EXEC = $(BUILD_DIR)/myapp

# Default target
all: $(EXEC)

# Link the executable
$(EXEC): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include dependency files
-include $(DEPS)

# Clean build artifacts (removes files but keeps build directory)
clean:
	rm -f $(BUILD_DIR)/*

# Phony targets
.PHONY: all clean

# Additional notes:
# - Uses rm -f $(BUILD_DIR)/* to remove all files in build directory
# - Preserves the build directory itself
# - Removes all object files, dependency files, and the executable