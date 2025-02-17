#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Find all test files in src/ directory
TEST_FILES=$(find src/ -name "test_*.cc")

if [ -z "$TEST_FILES" ]; then
    echo -e "${RED}No test files found in src/ directory${NC}"
    exit 1
fi

# Compile and run each test file
for test_file in $TEST_FILES; do
    test_name=$(basename "$test_file" .cc)
    echo -e "\n${GREEN}Running tests from: $test_file${NC}"
    
    # Compile with C++23 and link with gtest
    g++ -std=c++23 "$test_file" -o "$test_name" -lgtest -lgtest_main -pthread
    
    if [ $? -eq 0 ]; then
        # Run the test
        ./"$test_name"
        # Clean up the binary
        rm "$test_name"
    else
        echo -e "${RED}Compilation failed for $test_file${NC}"
    fi
done
