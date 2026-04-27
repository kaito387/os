#ifndef CONTEXT_H
#define CONTEXT_H

#include "thread.h"

// Save current thread's context
void save_context(thread_t *thread);

// Restore thread's context and resume execution
void restore_context(thread_t *thread);

// Switch from one thread to another
void thread_switch(thread_t *from, thread_t *to);

// Get current thread context for debugging
ThreadContext* get_current_context(void);

#endif
