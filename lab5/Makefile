# Makefile for Lab5 Printf Implementation
# Simple test to verify compilation

CXX = g++
CXXFLAGS = -g -Wall -march=i386 -m32 -nostdlib -fno-builtin -ffreestanding -fno-pic
INCLUDE = -I./include

# Source files
SOURCES = src/kernel/stdio.cpp src/kernel/printf.cpp src/kernel/thread.cpp src/utils/stdlib.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Test programs
TEST_SOURCES = test_printf.cpp test_thread_create.cpp
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)

# Targets
.PHONY: compile-printf compile-all test clean thread-test

# Compile just the printf and stdio modules
compile-printf: $(OBJECTS)
	@echo "Printf implementation compiled successfully"

# Compile all including test
compile-all: $(OBJECTS) $(TEST_OBJECTS)
	@echo "All source files compiled successfully"

# Compile thread test specifically
thread-test: src/kernel/thread.o test_thread_create.o
	@echo "Thread creation test compiled successfully"

# Compile object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS)
	@echo "Cleaned up object files"

# Help target
help:
	@echo "Available targets:"
	@echo "  compile-printf  - Compile printf and stdio implementation"
	@echo "  compile-all     - Compile all source files including test"
	@echo "  thread-test     - Compile thread creation test"
	@echo "  clean           - Remove compiled object files"
	@echo "  help            - Show this help message"
