#ifndef THREAD_H
#define THREAD_H

#include "os_type.h"
#include "os_constant.h"

typedef void (*ThreadFunction)(void *);

// Thread status enumeration
enum ThreadState {
    READY = 0,
    RUNNING = 1,
    BLOCKED = 2,
    DEAD = 3
};

// Process Control Block (PCB) / Thread descriptor
struct thread_t {
    uint32 *stack;                        // Stack pointer (esp) for context switching
    char name[MAX_PROGRAM_NAME + 1];      // Thread name
    ThreadState state;                    // Current thread status
    uint32 priority;                      // Thread priority
    uint32 tid;                           // Thread ID
    uint32 ticks;                         // Total time quantum for this thread
    uint32 ticks_passed;                  // Ticks already used
    
    // x86 register state (for context save/restore)
    uint32 eax;
    uint32 ebx;
    uint32 ecx;
    uint32 edx;
    uint32 esi;
    uint32 edi;
    uint32 esp;
    uint32 ebp;
    uint32 eip;
};

// Thread operations
thread_t* thread_create(ThreadFunction entry, void *arg, const char *name, uint32 priority);
void thread_destroy(thread_t *thread);
thread_t* thread_current(void);
void thread_set_current(thread_t *thread);
void thread_exit(void);

// Thread initialization
void thread_init(void);

#endif
