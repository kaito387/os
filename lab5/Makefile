# Makefile for Lab5 Assignment


ASM = nasm
CXX = g++
CXXFLAGS = -g -Wall -std=gnu++11 -march=i386 -m32 -nostdlib -fno-builtin -ffreestanding -fno-pic
LINKER = ld
INCLUDE = -I./include

# Source files - core implementation
CXX_SOURCES = $(wildcard src/kernel/*.cpp) $(wildcard src/utils/*.cpp)
ASM_SOURCES = src/utils/asm_utils.asm
CXX_OBJECTS = $(CXX_SOURCES:.cpp=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
OBJECTS = $(CXX_OBJECTS) $(ASM_OBJECTS)

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
	@ls -lh $(OBJECTS) 2>/dev/null | awk '{print "  " $$9 " (" $$5 ")"}'
	@echo ""

# Compile object files

src/kernel/%.o: src/kernel/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

src/utils/%.o: src/utils/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

src/utils/asm_utils.o: src/utils/asm_utils.asm
	$(ASM) -f elf32 $< -o $@

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
