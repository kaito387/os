#include "thread.h"
#include "stdio.h"
#include "os_constant.h"

// Global variables
static thread_t *current_thread = nullptr;
static uint32 next_tid = 1;

// Thread pool - pre-allocated thread structs
static char thread_pool[PCB_SIZE * MAX_PROGRAM_AMOUNT];
static bool thread_pool_used[MAX_PROGRAM_AMOUNT];

// Stack pool - pre-allocated stacks for threads
static char stack_pool[STACK_SIZE * MAX_PROGRAM_AMOUNT];
static bool stack_pool_used[MAX_PROGRAM_AMOUNT];

// Initialize thread system
void thread_init(void) {
    // Clear all thread pool entries
    for (int i = 0; i < MAX_PROGRAM_AMOUNT; i++) {
        thread_pool_used[i] = false;
        stack_pool_used[i] = false;
    }
    
    current_thread = nullptr;
    next_tid = 1;
    
    printf("Thread system initialized\n");
}

// Allocate a thread structure from the pool
static thread_t* thread_alloc(void) {
    for (int i = 0; i < MAX_PROGRAM_AMOUNT; i++) {
        if (!thread_pool_used[i]) {
            thread_pool_used[i] = true;
            thread_t *t = (thread_t *)&thread_pool[i * PCB_SIZE];
            // Clear the structure
            for (int j = 0; j < PCB_SIZE; j++) {
                ((char *)t)[j] = 0;
            }
            return t;
        }
    }
    return nullptr;
}

// Allocate a stack from the pool
static uint32* stack_alloc(void) {
    for (int i = 0; i < MAX_PROGRAM_AMOUNT; i++) {
        if (!stack_pool_used[i]) {
            stack_pool_used[i] = true;
            return (uint32 *)&stack_pool[(i + 1) * STACK_SIZE];  // Return end of stack
        }
    }
    return nullptr;
}

// Free a thread structure
static void thread_free(thread_t *t) {
    if (!t) return;
    
    // Find which pool entry this thread is in
    int idx = ((char *)t - thread_pool) / PCB_SIZE;
    if (idx >= 0 && idx < MAX_PROGRAM_AMOUNT) {
        thread_pool_used[idx] = false;
    }
}

// Free a stack
static void stack_free(uint32 *sp) {
    if (!sp) return;
    
    // Find which pool entry this stack is in (sp points to end of stack)
    int idx = ((char *)sp - stack_pool) / STACK_SIZE - 1;
    if (idx >= 0 && idx < MAX_PROGRAM_AMOUNT) {
        stack_pool_used[idx] = false;
    }
}

// Create a new thread
// entry: function to execute
// arg: argument to pass to entry function
// name: thread name (for debugging)
// priority: thread priority
thread_t* thread_create(ThreadFunction entry, void *arg, const char *name, uint32 priority) {
    // Allocate thread structure
    thread_t *thread = thread_alloc();
    if (!thread) {
        printf("ERROR: Failed to allocate thread structure\n");
        return nullptr;
    }
    
    // Allocate stack
    uint32 *stack_top = stack_alloc();
    if (!stack_top) {
        printf("ERROR: Failed to allocate thread stack\n");
        thread_free(thread);
        return nullptr;
    }
    
    // Initialize thread structure
    thread->tid = next_tid++;
    thread->state = READY;
    thread->priority = priority;
    thread->ticks = priority * 10;  // Time quantum based on priority
    thread->ticks_passed = 0;
    
    // Copy thread name
    if (name) {
        int i = 0;
        while (i < MAX_PROGRAM_NAME && name[i]) {
            thread->name[i] = name[i];
            i++;
        }
        thread->name[i] = '\0';
    }
    
    // Initialize stack
    // Stack grows downward in x86
    // We set up the stack so that when the thread starts, it will call entry(arg)
    // and when entry returns, it will call thread_exit()
    
    // Layout (from high to low address):
    // [stack_top - 0]: parameter to entry function
    // [stack_top - 1]: return address (thread_exit)
    // [stack_top - 2]: entry function address (pushed by call instruction)
    // [stack_top - 3]: ebx
    // [stack_top - 4]: ecx
    // [stack_top - 5]: edx
    // [stack_top - 6]: esi
    // [stack_top - 7]: edi
    // [stack_top - 8]: ebp
    // [stack_top - 9]: eax
    
    uint32 *sp = stack_top - 9;
    
    sp[8] = (uint32)arg;              // Parameter to entry function
    sp[7] = (uint32)thread_exit;      // Return address when entry returns
    sp[6] = (uint32)entry;            // Entry function address
    sp[5] = 0;                         // ebx = 0
    sp[4] = 0;                         // ecx = 0
    sp[3] = 0;                         // edx = 0
    sp[2] = 0;                         // esi = 0
    sp[1] = 0;                         // edi = 0
    sp[0] = 0;                         // ebp = 0 (frame pointer)
    sp[-1] = 0;                        // eax = 0
    
    thread->stack = sp - 1;            // esp points to eax position
    
    // Initialize registers
    thread->esp = (uint32)sp - 4;
    thread->ebp = 0;
    thread->eax = 0;
    thread->ebx = 0;
    thread->ecx = 0;
    thread->edx = 0;
    thread->esi = 0;
    thread->edi = 0;
    thread->eip = (uint32)entry;
    
    printf("Thread %s (ID: %d) created successfully\n", thread->name, thread->tid);
    
    return thread;
}

// Destroy a thread and free its resources
void thread_destroy(thread_t *thread) {
    if (!thread) return;
    
    printf("Thread %s (ID: %d) destroyed\n", thread->name, thread->tid);
    
    // In a real system, we would also free the stack
    // For now, we'll just mark it as available
    stack_free(thread->stack);
    thread_free(thread);
}

// Get current running thread
thread_t* thread_current(void) {
    return current_thread;
}

// Set current running thread
void thread_set_current(thread_t *thread) {
    current_thread = thread;
    if (thread) {
        thread->state = RUNNING;
    }
}

// Thread exit - should be called when thread entry function returns
void thread_exit(void) {
    printf("Thread exit\n");
    if (current_thread) {
        current_thread->state = DEAD;
    }
}
