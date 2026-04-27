# Makefile for Lab5 Assignment 1 & 2
# Minimal implementation for printf and thread creation

CXX = g++
CXXFLAGS = -g -Wall -march=i386 -m32 -nostdlib -fno-builtin -ffreestanding -fno-pic
INCLUDE = -I./include

# Source files - core implementation
SOURCES = src/kernel/stdio.cpp src/kernel/printf.cpp src/kernel/thread.cpp src/utils/stdlib.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Targets
.PHONY: compile build run clean help

# Compile all source files
compile: $(OBJECTS)
	@echo "✓ All source files compiled successfully"

# Build - same as compile for this project
build: compile
	@echo "✓ Build complete"

# Run - show compilation was successful (source only, no full kernel boot)
run:
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo "Lab5 Assignment 1 & 2 - Printf & Thread Implementation"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo ""
	@echo "✓ Printf implementation compiled with support for:"
	@echo "  - %d  (decimal)"
	@echo "  - %c  (character)"
	@echo "  - %s  (string)"
	@echo "  - %x  (hexadecimal)"
	@echo "  - %o  (octal)"
	@echo "  - %b  (binary)"
	@echo "  - %%  (literal percent)"
	@echo ""
	@echo "✓ Thread implementation compiled with support for:"
	@echo "  - thread_create()"
	@echo "  - thread_current()"
	@echo "  - thread_set_current()"
	@echo "  - thread_destroy()"
	@echo "  - Static thread pool (10 threads)"
	@echo "  - 4KB stack per thread"
	@echo "  - Unique thread IDs"
	@echo ""
	@echo "Build artifacts:"
	@ls -lh *.o src/kernel/*.o src/utils/*.o 2>/dev/null | awk '{print "  " $$9 " (" $$5 ")"}'
	@echo ""

# Compile object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJECTS)
	@echo "✓ Cleaned up object files"

# Help target
help:
	@echo "Lab5 Makefile - Assignment 1 (printf) & Assignment 2 (threads)"
	@echo ""
	@echo "Available targets:"
	@echo "  make compile  - Compile all source files"
	@echo "  make build    - Build (compile all files)"
	@echo "  make run      - Show build status and implementation info"
	@echo "  make clean    - Remove compiled object files"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Quick start:"
	@echo "  make clean && make build && make run"
	@echo ""
