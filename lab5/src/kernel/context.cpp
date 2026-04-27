#include "context.h"
#include "thread.h"
#include "stdio.h"

// Simple context switching implementation
// In a real kernel, this would involve assembly code for register manipulation

extern thread_t *currentThread;

void save_context(thread_t *thread) {
    if (thread == NULL) return;
    
    // In a real implementation, this would be done in assembly
    // to capture actual CPU registers at interrupt time
    // For now, just mark that we've saved state
    thread->state = READY;
}

void restore_context(thread_t *thread) {
    if (thread == NULL) return;
    
    thread->state = RUNNING;
    // In a real implementation, this would restore CPU registers
    // and resume execution from saved context
}

void thread_switch(thread_t *from, thread_t *to) {
    if (from == NULL || to == NULL) return;
    
    printf("Switching from thread %d to thread %d\n", from->id, to->id);
    
    // Save current thread
    save_context(from);
    
    // Restore next thread
    restore_context(to);
}

ThreadContext* get_current_context(void) {
    extern thread_t *currentThread;
    if (currentThread != NULL) {
        return &currentThread->context;
    }
    return NULL;
}
