# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -Wall -g
# Source directory
SRC_DIR = ./src
# Output directory
BUILD_DIR = ./build
# Entry point
ENTRY_POINT = $(SRC_DIR)/main.cc
# Excluded file
EXCLUDE_FILE = $(SRC_DIR)/client.cc

# Find all .cc files in the source directory, excluding the specified one
SOURCES = $(filter-out $(EXCLUDE_FILE), $(wildcard $(SRC_DIR)/*.cc))
# Output binary name
OUTPUT = $(BUILD_DIR)/openloadbalancer

# Rule to build the output binary
$(OUTPUT): $(SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(ENTRY_POINT) $(filter-out $(ENTRY_POINT), $(SOURCES))

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR)
