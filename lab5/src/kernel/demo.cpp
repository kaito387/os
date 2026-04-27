#include "stdio.h"
#include "os_type.h"
#include "thread.h"

// Global STDIO instance
extern STDIO stdio;

// Simple thread function for demo
void demo_thread_func(void *arg) {
    int id = (int)arg;
    printf("Thread %d running\n", id);
}

// Demonstrate printf with all format specifiers
void demo_printf() {
    printf("=== Printf Format Specifiers ===\n");
    printf("Print percentage: %%\n");
    printf("Print char 'A': %c\n", 'A');
    printf("Print string: %s\n", "Hello");
    printf("Print decimal: %d\n", -1234);
    printf("Print hex: %x\n", 0xFF);
    printf("Print octal: %o\n", 077);
    printf("Print binary: %b\n", 0b1010);
    printf("\n");
}

// Demonstrate thread creation
void demo_threads() {
    printf("=== Thread Creation Demo ===\n");
    
    thread_init();
    printf("Thread system initialized\n");
    
    thread_t *t1 = thread_create(demo_thread_func, (void *)1, "thread1", 10);
    thread_t *t2 = thread_create(demo_thread_func, (void *)2, "thread2", 8);
    
    if (t1 && t2) {
        printf("Thread 1 ID: %d\n", t1->tid);
        printf("Thread 2 ID: %d\n", t2->tid);
        printf("Threads created successfully\n");
    }
    printf("\n");
}
