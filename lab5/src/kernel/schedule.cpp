#include "schedule.h"
#include "stdio.h"
#include "stdlib.h"

#define MAX_READY_QUEUE 10

// Ready queue for FCFS scheduling
typedef struct {
    thread_t *threads[MAX_READY_QUEUE];
    uint32 head;
    uint32 tail;
    uint32 count;
} ReadyQueue;

static ReadyQueue readyQueue;
static uint32 currentScheduleAlgorithm = 0; // 0 = FCFS, 1 = RR

void schedule_init(void) {
    readyQueue.head = 0;
    readyQueue.tail = 0;
    readyQueue.count = 0;
}

void schedule_add_thread(thread_t *thread) {
    if (thread == NULL || readyQueue.count >= MAX_READY_QUEUE) {
        return;
    }
    
    readyQueue.threads[readyQueue.tail] = thread;
    readyQueue.tail = (readyQueue.tail + 1) % MAX_READY_QUEUE;
    readyQueue.count++;
    
    printf("Thread %d added to ready queue (count: %d)\n", thread->id, readyQueue.count);
}

void schedule_remove_thread(thread_t *thread) {
    if (thread == NULL || readyQueue.count == 0) {
        return;
    }
    
    // Simple implementation: find and remove
    for (uint32 i = 0; i < readyQueue.count; i++) {
        uint32 idx = (readyQueue.head + i) % MAX_READY_QUEUE;
        if (readyQueue.threads[idx] == thread) {
            // Shift elements
            for (uint32 j = i; j < readyQueue.count - 1; j++) {
                uint32 curr = (readyQueue.head + j) % MAX_READY_QUEUE;
                uint32 next = (readyQueue.head + j + 1) % MAX_READY_QUEUE;
                readyQueue.threads[curr] = readyQueue.threads[next];
            }
            readyQueue.count--;
            readyQueue.tail = (readyQueue.tail - 1 + MAX_READY_QUEUE) % MAX_READY_QUEUE;
            printf("Thread %d removed from ready queue\n", thread->id);
            return;
        }
    }
}

// FCFS scheduling: First Come First Served
thread_t* schedule_fcfs(void) {
    if (readyQueue.count == 0) {
        return NULL;
    }
    
    thread_t *next = readyQueue.threads[readyQueue.head];
    readyQueue.head = (readyQueue.head + 1) % MAX_READY_QUEUE;
    readyQueue.count--;
    
    printf("FCFS: Selected thread %d\n", next->id);
    return next;
}

// Round Robin scheduling
thread_t* schedule_rr(uint32 timeQuantum) {
    if (readyQueue.count == 0) {
        return NULL;
    }
    
    // Get next thread and move it to back of queue for round-robin
    thread_t *next = readyQueue.threads[readyQueue.head];
    readyQueue.head = (readyQueue.head + 1) % MAX_READY_QUEUE;
    
    // Re-add to back of queue
    readyQueue.threads[readyQueue.tail] = next;
    readyQueue.tail = (readyQueue.tail + 1) % MAX_READY_QUEUE;
    
    printf("RR: Selected thread %d\n", next->id);
    return next;
}

// Default scheduling function
thread_t* schedule_next(void) {
    if (currentScheduleAlgorithm == 0) {
        return schedule_fcfs();
    } else {
        return schedule_rr(10); // 10ms time quantum
    }
}
