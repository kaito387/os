# Lab5 Assignment 2: Thread Creation Implementation

## Overview
This document describes the implementation of thread creation for the Lab5 kernel threading system.

## Architecture

### Thread Structure (thread_t)
The `thread_t` structure (Process Control Block or PCB) contains:
- **Stack Management**: `stack` pointer to thread-local stack
- **Identification**: `tid` (thread ID) and `name` (thread name)
- **State Management**: `state` (READY, RUNNING, BLOCKED, DEAD)
- **Scheduling**: `priority`, `ticks`, `ticks_passed` for priority-based scheduling
- **Register State**: eax, ebx, ecx, edx, esi, edi, esp, ebp, eip for context switching

### Key Design Decisions

#### 1. **Static Memory Allocation**
- **Thread Pool**: `MAX_PROGRAM_AMOUNT` (10) pre-allocated 4KB thread structures
- **Stack Pool**: `MAX_PROGRAM_AMOUNT` pre-allocated 4KB stacks
- Advantages:
  - No fragmentation issues
  - Predictable memory layout
  - No malloc/free overhead
  - Suitable for embedded/kernel context
- Bitmap tracking (`thread_pool_used`, `stack_pool_used`) for allocation status

#### 2. **Stack Layout for x86**
Stack is arranged for x86 calling conventions:
```
High Address
   [arg]          <- thread entry parameter
   [ret addr]     <- points to thread_exit()
   [entry]        <- thread entry function
   [ebx]
   [ecx]
   [edx]
   [esi]
   [edi]
   [ebp]
   [eax]          <- esp points here
Low Address
```

#### 3. **Thread Initialization**
When `thread_create()` is called:
1. Allocate thread structure from pool
2. Allocate stack from pool
3. Initialize thread metadata (ID, name, priority, state)
4. Set up stack frame with entry point and argument
5. Initialize register state (all zeros initially)
6. Return thread descriptor

#### 4. **Thread-Local Storage**
- Global `current_thread` pointer tracks running thread
- `thread_set_current()` updates current thread (used by scheduler)
- `thread_current()` retrieves current thread

#### 5. **Priority-Based Time Quantum**
- Time quantum = `priority * 10` ticks
- Higher priority = more CPU time per scheduling round
- Allows for simple but effective scheduling

## Implementation Details

### File Organization
- **include/thread.h**: Thread structure and API declarations
- **src/kernel/thread.cpp**: Thread management implementation
- **include/os_constant.h**: Thread-related constants
- **test_thread_create.cpp**: Comprehensive test suite

### Key Functions

#### `thread_init()`
Initializes the thread system:
- Clears thread and stack pool allocation bitmaps
- Resets next thread ID to 1
- Prints initialization message

#### `thread_create(entry, arg, name, priority)`
Creates a new thread:
- Allocates thread structure and stack
- Initializes thread metadata
- Sets up stack frame with entry point and argument
- Returns thread descriptor pointer

#### `thread_set_current(thread)` / `thread_current()`
Manages current thread:
- `thread_set_current()`: Updates global current thread pointer and marks as RUNNING
- `thread_current()`: Returns current running thread

#### `thread_exit()`
Thread cleanup:
- Marks thread as DEAD
- Called when thread entry function returns
- In future implementation: triggers scheduler to remove thread

### Memory Layout

#### PCB Storage
```
thread_pool:
[0]: PCB[0] (4KB)
[1]: PCB[1] (4KB)
...
[9]: PCB[9] (4KB)
Total: 40KB
```

#### Stack Storage
```
stack_pool:
[0]: Stack[0] (4KB) - grows downward
[1]: Stack[1] (4KB) - grows downward
...
[9]: Stack[9] (4KB) - grows downward
Total: 40KB
```

Each stack pointer points to the END of its 4KB block and grows downward.

## Compilation

### Build Commands
```bash
# Compile just thread support
make thread-test

# Compile all modules
make compile-all

# Clean object files
make clean
```

### Compilation Flags
- `-march=i386 -m32`: 32-bit x86 architecture
- `-nostdlib -fno-builtin -ffreestanding`: Kernel/embedded environment
- `-fno-pic`: Position-independent code not needed

## Testing

### Test Program (test_thread_create.cpp)
Demonstrates:
1. **Thread Creation**: Creates 3 threads with different names and priorities
2. **Stack Verification**: Confirms each thread has unique stack
3. **Thread ID Uniqueness**: Verifies all threads have different IDs
4. **Current Thread Management**: Tests set/get current thread
5. **Thread Properties**: Displays thread metadata

### Test Output Example
```
=== Thread Creation Test ===
Thread system initialized
Thread thread1 (ID: 1) created successfully
Successfully created thread1
  Thread ID: 1
  Thread Name: thread1
  Thread State: 2
  Thread Priority: 10
  Stack pointer: 0x...

[Additional threads created...]

=== Stack Verification ===
SUCCESS: Each thread has its own unique stack

=== Thread ID Uniqueness Test ===
SUCCESS: All thread IDs are unique
```

## Thread States
- **READY (0)**: Thread is ready to run but not currently executing
- **RUNNING (1)**: Thread is currently executing
- **BLOCKED (2)**: Thread is waiting for a resource
- **DEAD (3)**: Thread has finished execution

## Thread Scheduling (Future Work)
Current implementation provides thread creation foundation. Scheduler will:
1. Select next READY thread based on priority
2. Save current thread's register state
3. Load next thread's register state
4. Transfer control using context switch
5. Update thread states appropriately

## Limitations and Future Improvements
1. **No Inter-thread Communication**: No mutexes, semaphores, or message passing yet
2. **Fixed Pool Size**: Only 10 threads maximum
3. **No Thread Termination**: threads can't be destroyed during scheduling
4. **No Priority Inheritance**: No protection against priority inversion
5. **No Per-Thread Errno**: Shared errno between threads
6. **Passive Current Thread**: Scheduler must explicitly call `thread_set_current()`

## References
- Lab5 reference implementation in materials/实验5_相关材料/src/4/
- x86 calling conventions and stack frame layout
- OS kernel design principles for embedded systems
