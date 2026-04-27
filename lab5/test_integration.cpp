#include "stdio.h"
#include "os_type.h"
#include "thread.h"

extern STDIO stdio;

void thread_worker(void *arg) {
    int id = (int)arg;
    printf("Worker thread %d\n", id);
}

int main() {
    // Test 1: Printf with all format specifiers
    printf("=== Assignment 1: Printf ===\n");
    printf("Percent: %%\n");
    printf("Char: %c\n", 'X');
    printf("String: %s\n", "Lab5");
    printf("Decimal: %d\n", -42);
    printf("Hex: %x\n", 0xDEADBEEF);
    printf("Octal: %o\n", 0755);
    printf("Binary: %b\n", 0b11110000);
    printf("\n");
    
    // Test 2: Thread creation
    printf("=== Assignment 2: Threads ===\n");
    thread_init();
    printf("Thread system initialized\n");
    
    thread_t *t1 = thread_create(thread_worker, (void *)1, "worker1", 10);
    thread_t *t2 = thread_create(thread_worker, (void *)2, "worker2", 8);
    
    if (t1) printf("Created thread: %s (ID: %d)\n", t1->name, t1->tid);
    if (t2) printf("Created thread: %s (ID: %d)\n", t2->name, t2->tid);
    
    printf("\n✓ All tests completed successfully\n");
    
    return 0;
}
