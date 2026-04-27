# PCB Structure Research - Lab5 Assignment 2 (Thread Implementation)

## Executive Summary

After examining the Lab5 kernel implementation materials (specifically `src/3` and `src/4` reference implementation), I've documented the complete PCB (Process Control Block) structure and thread management system needed for Lab5 Assignment 2.

**Examined files:**
- `/home/lht/dev/study/os/lab5/materials/实验5_相关材料/src/3/include/` - baseline kernel headers
- `/home/lht/dev/study/os/lab5/materials/实验5_相关材料/src/4/include/thread.h` - PCB structure definition
- `/home/lht/dev/study/os/lab5/materials/实验5_相关材料/src/4/src/kernel/program.cpp` - thread creation and scheduling
- `/home/lht/dev/study/os/lab5/materials/实验5_相关材料/src/4/src/kernel/interrupt.cpp` - interrupt-driven scheduling
- `/home/lht/dev/study/os/lab5/materials/实验5_相关材料/src/4/src/utils/asm_utils.asm` - context switch assembly

---

## 1. PCB Structure Fields

The PCB struct (`thread.h:18-29`) contains:

```cpp
struct PCB {
    int *stack;                      // Stack pointer (ESP) - only thing stored in PCB
    char name[MAX_PROGRAM_NAME + 1]; // Thread identifier string
    enum ProgramStatus status;       // Current state (CREATED/RUNNING/READY/BLOCKED/DEAD)
    int priority;                    // Priority level (1-10, lower = lower priority)
    int pid;                         // Thread identifier (0 = main thread)
    int ticks;                       // Remaining ticks in current time slice
    int ticksPassedBy;               // Total time executed so far
    ListItem tagInGeneralList;       // Node in ready queue (intrusive list)
    ListItem tagInAllList;           // Node in all-threads list (intrusive list)
};
```

### Key Fields Explained:
- **`stack`**: Only register context saved in PCB is ESP. All other registers pushed/popped on stack.
- **`ticks`**: Initialized to `priority * 10`. Decremented each timer interrupt. When 0, triggers scheduling.
- **`tagInGeneralList` & `tagInAllList`**: Embedded list nodes - no separate allocations needed.

---

## 2. Thread Stack Architecture

### Memory Layout

Each thread gets a 4KB page allocated from `PCB_SET[PCB_SIZE * MAX_PROGRAM_AMOUNT]` pool.

```
PCB Page Layout (4096 bytes)
┌─────────────────────────────────────┐
│                                     │ 0x0000
│      PCB Structure (~96 bytes)      │
│                                     │
├─────────────────────────────────────┤
│                                     │
│   Available for Stack Growth        │
│   (grows downward from top)         │
│                                     │
│                                     │
└─────────────────────────────────────┤ 0x1000 (4096 bytes)
      └─ ESP points here initially
```

### Stack Initialization for New Thread

When `executeThread()` creates a thread, it initializes the stack pointer at the end of the PCB page:

```cpp
thread->stack = (int *)((int)thread + PCB_SIZE);  // Point to end
thread->stack -= 7;                                // Back up 7 words (28 bytes)

// Stack layout after initialization (top to bottom):
stack[0] = 0;                    // Placeholder for EBP push
stack[1] = 0;                    // Placeholder for EBX push
stack[2] = 0;                    // Placeholder for EDI push  
stack[3] = 0;                    // Placeholder for ESI push
stack[4] = (int)function;        // Thread entry point (will be called by ret)
stack[5] = (int)program_exit;    // Return address (cleanup handler)
stack[6] = (int)parameter;       // Thread function parameter
```

**Why 7 words?**
- 4 words for caller-saved registers (EBP, EBX, EDI, ESI)
- 1 word for function pointer
- 1 word for return address
- 1 word for parameter

---

## 3. Register Context Save/Restore

### Registers Saved During Context Switch

Only non-volatile (callee-saved) registers are stored:
- **EBP** - Base Pointer
- **EBX** - Base Register
- **EDI** - Destination Index
- **ESI** - Source Index

Volatile registers (EAX, ECX, EDX) are NOT saved - caller's responsibility.

### Assembly Context Switch (`asm_utils.asm:23-41`)

```asm
asm_switch_thread:
    push ebp        ; Save caller-saved register
    push ebx
    push edi
    push esi
    
    mov eax, [esp + 5*4]  ; Get *cur (current PCB)
    mov [eax], esp        ; Save current ESP → cur->stack
    
    mov eax, [esp + 6*4]  ; Get *next (next PCB)
    mov esp, [eax]        ; Restore next ESP ← next->stack
    
    pop esi         ; Restore next thread's registers
    pop edi
    pop ebx
    pop ebp
    
    sti             ; Enable interrupts (in next thread's context)
    ret             ; Return to next thread
```

**Key insight**: Only ESP is stored in PCB. All registers live on the stack!

---

## 4. Per-Thread State Management

### Thread States (`thread.h:9-16`)

```cpp
enum ProgramStatus {
    CREATED  = 0,  // Created but not yet ready
    RUNNING  = 1,  // Currently executing
    READY    = 2,  // Ready to run, in queue
    BLOCKED  = 3,  // Waiting for event/I/O
    DEAD     = 4   // Finished execution
};
```

### State Transitions

```
CREATED → READY (added to ready queue)
         ↓
       RUNNING (scheduler selected it)
         ↓
  ┌─────┴─────┐
  ↓           ↓
READY      BLOCKED (time slice expired) (waiting for event)
 ↑           ↓
 └───────────┘ (event occurs / time slice allocated)
  
RUNNING → DEAD (function returns)
           ↓
      (releasePCB called)
```

### Time Slice Management

```cpp
// During thread creation
thread->ticks = priority * 10;  // Priority 1 → 10 ticks, Priority 10 → 100 ticks

// Timer interrupt handler (every 10ms typically)
if (cur->ticks) {
    --cur->ticks;        // Decrement remaining time
    ++cur->ticksPassedBy; // Track cumulative usage
} else {
    schedule();          // Time expired, invoke scheduler
}

// Scheduler resets time slice
running->ticks = running->priority * 10;  // Fresh slice
```

---

## 5. Thread Creation Lifecycle

### Step-by-step Creation (`program.cpp:30-74`)

```cpp
int executeThread(ThreadFunction function, void *parameter, 
                  const char *name, int priority) {
    // 1. Disable interrupts (atomic operation)
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();
    
    // 2. Allocate 4KB page from PCB pool
    PCB *thread = allocatePCB();  // Returns &PCB_SET[i*4096]
    
    // 3. Zero-initialize entire page
    memset(thread, 0, PCB_SIZE);
    
    // 4. Set PCB fields
    strcpy(thread->name, name);
    thread->status = ProgramStatus::READY;
    thread->priority = priority;
    thread->ticks = priority * 10;
    thread->ticksPassedBy = 0;
    thread->pid = ((int)thread - (int)PCB_SET) / PCB_SIZE;
    
    // 5. Initialize stack (top of page, grows downward)
    thread->stack = (int *)((int)thread + PCB_SIZE);
    thread->stack -= 7;
    thread->stack[0] = 0;
    thread->stack[1] = 0;
    thread->stack[2] = 0;
    thread->stack[3] = 0;
    thread->stack[4] = (int)function;      // Entry point
    thread->stack[5] = (int)program_exit;  // Cleanup
    thread->stack[6] = (int)parameter;     // Arg to function
    
    // 6. Add to queues
    allPrograms.push_back(&(thread->tagInAllList));
    readyPrograms.push_back(&(thread->tagInGeneralList));
    
    // 7. Restore interrupts
    interruptManager.setInterruptStatus(status);
    
    return thread->pid;
}
```

---

## 6. Scheduler Design

### Round-Robin with Priority

The scheduler is interrupt-driven and implements round-robin scheduling:

```cpp
extern "C" void c_time_interrupt_handler() {
    PCB *cur = programManager.running;
    
    if (cur->ticks) {
        --cur->ticks;        // Decrement time slice
        ++cur->ticksPassedBy;
    } else {
        programManager.schedule();  // Time's up, reschedule
    }
}

void schedule() {
    // 1. Save current thread state
    if (running->status == RUNNING) {
        running->status = READY;
        running->ticks = running->priority * 10;  // Reset time
        readyPrograms.push_back(&running->tagInGeneralList);
    }
    
    // 2. Handle dead threads
    if (running->status == DEAD) {
        releasePCB(running);
    }
    
    // 3. Select next thread from ready queue
    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item, tagInGeneralList);
    next->status = RUNNING;
    running = next;
    readyPrograms.pop_front();
    
    // 4. Context switch
    asm_switch_thread(cur, next);
}
```

### Queue Structure

Uses intrusive doubly-linked lists:

```cpp
List readyPrograms;  // Ready to run threads
List allPrograms;    // All created threads

// Container macro - convert ListItem* to PCB*
#define ListItem2PCB(ADDRESS, LIST_ITEM) \
    ((PCB *)((int)(ADDRESS) - (int)&((PCB *)0)->LIST_ITEM))
```

---

## 7. Thread Termination

### Cleanup Flow

```cpp
void program_exit() {
    PCB *thread = programManager.running;
    thread->status = ProgramStatus::DEAD;
    
    if (thread->pid) {
        // Regular thread: reschedule
        programManager.schedule();
    } else {
        // Main thread (pid=0): system halt
        interruptManager.disableInterrupt();
        printf("halt\n");
        asm_halt();
    }
}

// PCB deallocation
void releasePCB(PCB *program) {
    int index = ((int)program - (int)PCB_SET) / PCB_SIZE;
    PCB_SET_STATUS[index] = false;  // Mark as available
}
```

---

## 8. Memory Constants

```cpp
// os_constant.h
#define IDT_START_ADDRESS 0x8880    // Interrupt Descriptor Table location
#define CODE_SELECTOR 0x20          // Protected mode code segment
#define MAX_PROGRAM_NAME 16         // Thread name max length
#define MAX_PROGRAM_AMOUNT 16       // Max concurrent threads

// program.cpp
const int PCB_SIZE = 4096;          // Each thread gets 4KB
const char PCB_SET[4096*16] = ...;  // 64KB pool for 16 threads
const bool PCB_SET_STATUS[16] = ... // Allocation tracking
```

---

## 9. Key Design Decisions

### Design Choice 1: Stack at PCB Top
- **Why**: Efficient memory use - single 4KB page contains both PCB and stack
- **How**: PCB at bottom, stack grows down from top
- **Benefit**: No separate stack allocation, fast context switch

### Design Choice 2: Only ESP Stored
- **Why**: Minimal state to save during context switch
- **How**: All registers pushed/popped on stack
- **Benefit**: Fast context switch (only 4 register pushes + ESP copy)

### Design Choice 3: Intrusive Lists
- **Why**: Avoid dynamic memory allocation within kernel
- **How**: List nodes embedded in PCB structure
- **Benefit**: Thread can be in multiple lists simultaneously

### Design Choice 4: Time Slice = Priority * 10
- **Why**: Simple priority implementation
- **How**: Higher priority = longer time slice
- **Benefit**: Fair scheduling with priority preference

### Design Choice 5: Interrupt-Driven Scheduling
- **Why**: Preemptive multitasking
- **How**: Timer interrupt triggers scheduler
- **Benefit**: No thread starves CPU indefinitely

---

## 10. Integration Summary

### What Lab5 Assignment 2 Needs:

1. ✅ **PCB struct** with all 8 fields
2. ✅ **Stack initialization** - 7 words setup
3. ✅ **Context switch assembly** - save/restore 4 registers + ESP
4. ✅ **Thread creation** - allocate, initialize, queue
5. ✅ **Scheduling** - round-robin from ready queue
6. ✅ **Interrupt integration** - timer interrupt triggers scheduling
7. ✅ **Thread termination** - cleanup and reschedule
8. ✅ **Queue management** - dual queues for ready/all threads

---

## Source Files Reference

| File | Location | Content |
|------|----------|---------|
| `thread.h` | `src/4/include/` | PCB structure, ProgramStatus enum |
| `program.h` | `src/4/include/` | ProgramManager class declaration |
| `program.cpp` | `src/4/src/kernel/` | Thread creation, scheduling, allocation |
| `interrupt.cpp` | `src/4/src/kernel/` | Timer interrupt handler, scheduling trigger |
| `asm_utils.asm` | `src/4/src/utils/` | Context switch implementation |
| `list.h` | `src/4/include/` | Queue data structure |

---

## Quick Reference: Essential Code Snippets

### PCB Definition
```cpp
struct PCB {
    int *stack;
    char name[MAX_PROGRAM_NAME + 1];
    enum ProgramStatus status;
    int priority;
    int pid;
    int ticks;
    int ticksPassedBy;
    ListItem tagInGeneralList;
    ListItem tagInAllList;
};
```

### Stack Setup (New Thread)
```cpp
thread->stack = (int *)((int)thread + PCB_SIZE);
thread->stack -= 7;
thread->stack[4] = (int)thread_function;
thread->stack[5] = (int)program_exit;
thread->stack[6] = (int)parameter;
```

### Context Switch Assembly
```asm
asm_switch_thread:
    push ebp; push ebx; push edi; push esi
    mov eax, [esp + 5*4]; mov [eax], esp   ; Save cur
    mov eax, [esp + 6*4]; mov esp, [eax]   ; Restore next
    pop esi; pop edi; pop ebx; pop ebp
    sti; ret
```

### Thread Creation
```cpp
int pid = programManager.executeThread(
    thread_function,  // void (*)(void*)
    parameter_ptr,    // void*
    "thread_name",    // const char*
    priority          // int (1-10)
);
```

---

## Document Generated: 2024
Source: Lab5 Material Analysis - `src/3` and `src/4` reference implementations
