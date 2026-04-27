#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "thread.h"

// Scheduling queue operations
void schedule_init(void);
void schedule_add_thread(thread_t *thread);
void schedule_remove_thread(thread_t *thread);
thread_t* schedule_next(void);

// Different scheduling algorithms
// FCFS: First Come First Served
thread_t* schedule_fcfs(void);

// Round Robin with time quantum
thread_t* schedule_rr(uint32 timeQuantum);

#endif
