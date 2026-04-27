# PCB Structure Research - Lab5 Assignment 2 Thread Implementation

## Overview

This research package provides a complete understanding of the Process Control Block (PCB) structure needed to implement kernel threads for Lab5 Assignment 2. The research is based on examination of the Lab5 material source code, specifically the reference implementation in `src/4`.

**Research Date:** April 27, 2024  
**Location:** `/home/lht/dev/study/os/`

---

## Research Deliverables

This package includes three comprehensive documents:

### 1. **PCB_RESEARCH_FINDINGS.md** (440 lines, 14 KB)
**Comprehensive Technical Reference**

The main research document containing in-depth analysis of all PCB-related aspects:

- **Section 1**: PCB Structure Definition (all 8 fields explained)
- **Section 2**: Thread State Machine (5 states, transitions)
- **Section 3**: Memory Layout and Stack Management (4KB page layout)
- **Section 4**: Register Context and Context Switching (assembly patterns)
- **Section 5**: Thread Creation Process (step-by-step walkthrough)
- **Section 6**: Thread Scheduling (round-robin algorithm)
- **Section 7**: Thread Termination (cleanup process)
- **Section 8**: Queue Management (intrusive lists)
- **Section 9**: Per-Thread Initialization Example
- **Section 10**: Key Design Insights (10 critical findings)

**Best for:** Understanding the "why" and "how" of PCB design

---

### 2. **PCB_QUICK_REFERENCE.txt** (290 lines, 13 KB)
**Quick Lookup and Visual Guide**

Quick reference document with visual diagrams and tables:

- **[1]** PCB Structure Definition (struct layout)
- **[2]** Memory Layout Per Thread (ASCII diagram)
- **[3]** Stack Initialization for New Thread (layout diagram)
- **[4]** Context Switch Register Layout (saved registers)
- **[5]** Thread States & Transitions (state machine diagram)
- **[6]** Time Slicing Mechanism (tick management)
- **[7]** Thread Creation Process (9-step flow)
- **[8]** Round-Robin Scheduling (algorithm diagram)
- **[9]** Queue Management (structure explanation)
- **[10]** Thread Termination (flow diagram)
- **[11]** Key Design Insights (11 key points)
- **[12]** Constants Reference (os_constant.h values)
- **[13]** Essential Files Reference (source file list)
- **Implementation Checklist** (14-item to-do list)

**Best for:** During coding, quick lookups, reference tables

---

### 3. **PCB_STUDY_SUMMARY.txt** (459 lines, 17 KB)
**Executive Summary and Roadmap**

High-level overview with implementation roadmap:

- **Key Findings** (consolidated summary)
- **10 Detailed Sections:**
  1. PCB/thread_t Structure Fields
  2. Thread Stack Allocation & Management
  3. Register Layout & Context Save/Restore
  4. Per-Thread State Management
  5. Thread Creation Requirements
  6. Scheduler & Interrupt Integration
  7. Thread Termination & Cleanup
  8. Queue Management
  9. Critical Design Insights
  10. Summary Table
  
- **Implementation Roadmap** (7 major tasks with sub-tasks)
- **Reference Materials** (source files and documents)

**Best for:** Getting oriented, implementation planning

---

## Quick Start Guide

### For First-Time Review
1. Start with **PCB_STUDY_SUMMARY.txt** (Executive Summary)
2. Skim **PCB_QUICK_REFERENCE.txt** (Visual diagrams)
3. Deep dive into **PCB_RESEARCH_FINDINGS.md** (Detailed analysis)

### For Implementation
1. Keep **PCB_QUICK_REFERENCE.txt** open during coding
2. Reference specific sections in **PCB_RESEARCH_FINDINGS.md** as needed
3. Follow the **Implementation Roadmap** in **PCB_STUDY_SUMMARY.txt**

### For Debugging
1. Consult **PCB_QUICK_REFERENCE.txt** [5] for register layout
2. Check **PCB_RESEARCH_FINDINGS.md** Section 4 for context switch
3. Review **PCB_STUDY_SUMMARY.txt** Section 6 for scheduler logic

---

## Key Findings Summary

### The 8 PCB Fields You Need

```cpp
struct PCB {
    int *stack;                      // Stack pointer (ESP only!)
    char name[17];                   // Thread name identifier
    enum ProgramStatus status;       // Current thread state
    int priority;                    // Priority level (1-10)
    int pid;                         // Process/thread ID
    int ticks;                       // Time slice countdown
    int ticksPassedBy;               // Cumulative execution time
    ListItem tagInGeneralList;       // Ready queue node
    ListItem tagInAllList;           // All threads list node
};
```

### The 3 Critical Insights

1. **Stack at PCB Top**
   - Both PCB and stack fit in one 4KB page
   - Stack grows downward from top
   - No separate allocations needed

2. **Only ESP Stored**
   - Only stack pointer (ESP) saved in PCB
   - All other registers on the stack
   - Minimal state = fast context switch

3. **Time Slice = Priority × 10**
   - Higher priority = longer turn
   - Fair round-robin scheduling
   - Preemptive via timer interrupt

### The Context Switch in 3 Steps

```asm
1. Save registers and current ESP
   push ebp; push ebx; push edi; push esi
   cur->stack = ESP

2. Restore next thread's ESP
   ESP = next->stack

3. Restore registers and return
   pop esi; pop edi; pop ebx; pop ebp
   sti; ret
```

---

## Source Material Analysis

### Files Examined

**From `/home/lht/dev/study/os/lab5/materials/实验5_相关材料/src/3/` (baseline):**
- `include/os_type.h` - Type definitions
- `include/interrupt.h` - Interrupt manager interface
- `include/asm_utils.h` - Assembly function declarations
- `src/kernel/interrupt.cpp` - Interrupt handling
- `src/kernel/setup.cpp` - Kernel initialization

**From `src/4/` (reference implementation):**
- `include/thread.h` - **PCB struct definition** ⭐
- `include/program.h` - **Thread manager interface** ⭐
- `include/list.h` - Doubly-linked list for queues
- `include/interrupt.h` - Extended interrupt manager
- `src/kernel/program.cpp` - **Thread creation/scheduling** ⭐
- `src/kernel/interrupt.cpp` - **Timer interrupt handler** ⭐
- `src/kernel/setup.cpp` - System initialization
- `src/utils/asm_utils.asm` - **Context switch assembly** ⭐

(⭐ = Critical for understanding PCB)

---

## Implementation Checklist

When implementing Lab5 Assignment 2, ensure you have:

- [ ] PCB struct with all 8 fields
- [ ] Thread state enum (5 states)
- [ ] Stack initialization (7-word setup)
- [ ] Context switch assembly (save/restore 4 registers)
- [ ] Thread creation function (allocate → initialize → queue)
- [ ] Scheduler logic (round-robin with time slices)
- [ ] Timer interrupt handler (tick decrement → scheduling)
- [ ] Thread termination (DEAD → cleanup → reschedule)
- [ ] Dual queue management (ready + all threads)
- [ ] Intrusive list implementation
- [ ] PCB allocation from pool
- [ ] Interrupt-safe operations (disable during critical sections)
- [ ] Testing (single thread, multiple threads, priority scheduling)

---

## Memory Layout Reference

```
PCB Memory Layout (4096 bytes per thread):

┌─────────────────────────────────────┐
│     PCB Structure (~96 bytes)       │ ← 0x0000
│  (name, status, priority, etc.)     │
├─────────────────────────────────────┤
│                                     │
│   Available Stack Space             │
│   (grows downward)                  │
│                                     │
│   [ESP+24] parameter                │
│   [ESP+20] program_exit address     │
│   [ESP+16] thread_function address  │
│   [ESP+12] reserved                 │
│   [ESP+8]  reserved                 │
│   [ESP+4]  reserved                 │
│   [ESP]    reserved                 │
│                                     │
└─────────────────────────────────────┘ ← 0x1000 (4096)
        ↑
    ESP points here initially
```

---

## Constants You'll Need

```cpp
#define MAX_PROGRAM_NAME 16         // Thread name max length
#define MAX_PROGRAM_AMOUNT 16       // Max concurrent threads
#define PCB_SIZE 4096               // Per-thread allocation
#define IDT_START_ADDRESS 0x8880    // Interrupt descriptor table
#define CODE_SELECTOR 0x20          // Protected mode code segment
```

---

## Scheduling Algorithm at a Glance

```
Timer Interrupt (every tick)
         ↓
  Is time slice > 0?
  ├─ YES → Decrement, continue running
  └─ NO  → Call scheduler
           ↓
           Save current thread → READY
           Get next ready thread
           Set to RUNNING
           Call context switch
           ↓ (next thread now running)
```

---

## Thread Creation at a Glance

```
executeThread(function, parameter, name, priority)
  ↓
1. Allocate PCB from pool
2. Initialize PCB fields
3. Setup 7-word stack frame
4. Add to queues
5. Return thread ID
```

---

## Thread Lifecycle

```
CREATE → READY → RUNNING ⇄ (time slice)
                   ↓
                  DEAD → CLEANUP
                (return)
```

---

## FAQ

**Q: Why is only ESP stored in PCB?**
A: The stack pointer is the only state that needs to be saved. All registers can be pushed/popped on the stack, making context switch fast and minimal.

**Q: Why 7 words in stack initialization?**
A: 4 for callee-saved registers, 1 for entry function, 1 for return address, 1 for parameter. This allows the thread to start executing with one `ret` instruction.

**Q: How is fairness ensured?**
A: All threads get a time slice proportional to priority. Round-robin scheduling ensures no thread starves, while time slices prevent one thread from monopolizing CPU.

**Q: What if a thread never yields?**
A: The timer interrupt forces a context switch when the time slice expires. This is preemptive scheduling—no thread can hold CPU indefinitely.

**Q: Can threads block?**
A: Yes, the BLOCKED state is defined. Implementation would require synchronization primitives (mutexes, semaphores, etc.) beyond the scope of this basic version.

---

## Testing Strategy

### Test 1: Single Thread
- Create one thread
- Verify it executes correctly
- Check it terminates properly

### Test 2: Two Threads
- Create two threads with priority 1
- Verify they alternate execution
- Check both complete successfully

### Test 3: Priority Scheduling
- Create threads with different priorities
- Verify higher priority threads run longer
- Check lower priority threads still run

### Test 4: Starvation Prevention
- Create many threads
- Verify all eventually execute
- Check no thread is starved

---

## References

### Intel x86 Architecture
- Calling conventions (cdecl)
- Register usage (callee-saved vs caller-saved)
- Protected mode memory
- Interrupt handling

### Required Reading
- PCB_RESEARCH_FINDINGS.md (comprehensive)
- PCB_QUICK_REFERENCE.txt (quick lookup)
- PCB_STUDY_SUMMARY.txt (overview)

### Source Code
- `/home/lht/dev/study/os/lab5/materials/实验5_相关材料/src/4/`

---

## Document Map

```
README_PCB_RESEARCH.md (this file)
├── PCB_RESEARCH_FINDINGS.md
│   ├── PCB Structure Definition
│   ├── Memory Layout
│   ├── Register Context
│   ├── Thread Creation
│   ├── Scheduling Algorithm
│   ├── Termination
│   ├── Queue Management
│   └── Design Insights
├── PCB_QUICK_REFERENCE.txt
│   ├── Structure Definition
│   ├── Memory Layout Diagram
│   ├── Stack Initialization
│   ├── Register Layout
│   ├── State Machine
│   ├── Time Slicing
│   ├── Creation Process
│   ├── Scheduling
│   ├── Queue Management
│   ├── Termination
│   ├── Design Insights
│   ├── Constants
│   └── Implementation Checklist
└── PCB_STUDY_SUMMARY.txt
    ├── Executive Summary
    ├── Key Findings
    ├── 10 Detailed Sections
    ├── Summary Tables
    ├── Implementation Roadmap
    └── Next Steps
```

---

## Success Metrics

After studying these materials, you should be able to answer:

1. ✓ What are the 8 fields in a PCB?
2. ✓ How is a thread's stack initialized?
3. ✓ Which registers are saved during context switch?
4. ✓ How is time slicing implemented?
5. ✓ What triggers the scheduler?
6. ✓ How do threads terminate?
7. ✓ Why are intrusive lists used?
8. ✓ How is memory allocated for PCBs?
9. ✓ How does round-robin scheduling work?
10. ✓ What makes this design efficient?

---

## Questions or Issues?

1. **Unclear on PCB structure?** → Review PCB_RESEARCH_FINDINGS.md Section 1
2. **Need quick lookup?** → Check PCB_QUICK_REFERENCE.txt
3. **Lost in implementation?** → Follow roadmap in PCB_STUDY_SUMMARY.txt
4. **Confused about context switch?** → See PCB_QUICK_REFERENCE.txt [4]
5. **Want overview?** → Read PCB_STUDY_SUMMARY.txt

---

**Research Completed:** April 27, 2024  
**Total Documentation:** 1,189 lines across 3 documents  
**Ready for Implementation:** ✓

Good luck with Lab5 Assignment 2!
