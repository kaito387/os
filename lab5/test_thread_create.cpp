#include "thread.h"
#include "stdio.h"

// Test thread function 1
void test_thread_1(void *arg) {
    int thread_num = (int)arg;
    printf("Thread 1 executing (number: %d)\n", thread_num);
    printf("Thread 1: Running in own context\n");
}

// Test thread function 2
void test_thread_2(void *arg) {
    int thread_num = (int)arg;
    printf("Thread 2 executing (number: %d)\n", thread_num);
    printf("Thread 2: Running in own context\n");
}

// Test thread function 3
void test_thread_3(void *arg) {
    int thread_num = (int)arg;
    printf("Thread 3 executing (number: %d)\n", thread_num);
    printf("Thread 3: Running in own context\n");
}

void test_thread_creation(void) {
    printf("\n=== Thread Creation Test ===\n");
    
    // Initialize thread system
    thread_init();
    
    // Create thread 1
    thread_t *t1 = thread_create(test_thread_1, (void *)1, "thread1", 10);
    if (t1) {
        printf("Successfully created thread1\n");
        printf("  Thread ID: %d\n", t1->tid);
        printf("  Thread Name: %s\n", t1->name);
        printf("  Thread State: %d\n", t1->state);
        printf("  Thread Priority: %d\n", t1->priority);
        printf("  Stack pointer: 0x%x\n", (uint32)t1->stack);
    } else {
        printf("ERROR: Failed to create thread1\n");
    }
    
    // Create thread 2
    thread_t *t2 = thread_create(test_thread_2, (void *)2, "thread2", 8);
    if (t2) {
        printf("\nSuccessfully created thread2\n");
        printf("  Thread ID: %d\n", t2->tid);
        printf("  Thread Name: %s\n", t2->name);
        printf("  Thread State: %d\n", t2->state);
        printf("  Thread Priority: %d\n", t2->priority);
        printf("  Stack pointer: 0x%x\n", (uint32)t2->stack);
    } else {
        printf("ERROR: Failed to create thread2\n");
    }
    
    // Create thread 3
    thread_t *t3 = thread_create(test_thread_3, (void *)3, "thread3", 5);
    if (t3) {
        printf("\nSuccessfully created thread3\n");
        printf("  Thread ID: %d\n", t3->tid);
        printf("  Thread Name: %s\n", t3->name);
        printf("  Thread State: %d\n", t3->state);
        printf("  Thread Priority: %d\n", t3->priority);
        printf("  Stack pointer: 0x%x\n", (uint32)t3->stack);
    } else {
        printf("ERROR: Failed to create thread3\n");
    }
    
    // Verify threads have different stacks
    if (t1 && t2 && t3) {
        printf("\n=== Stack Verification ===\n");
        printf("Thread1 stack: 0x%x\n", (uint32)t1->stack);
        printf("Thread2 stack: 0x%x\n", (uint32)t2->stack);
        printf("Thread3 stack: 0x%x\n", (uint32)t3->stack);
        
        if (t1->stack != t2->stack && t2->stack != t3->stack && t1->stack != t3->stack) {
            printf("SUCCESS: Each thread has its own unique stack\n");
        } else {
            printf("ERROR: Threads share stack memory!\n");
        }
    }
    
    // Test thread current functionality
    printf("\n=== Current Thread Test ===\n");
    thread_set_current(t1);
    thread_t *curr = thread_current();
    if (curr == t1) {
        printf("SUCCESS: Set and retrieved current thread correctly\n");
        printf("Current thread ID: %d, Name: %s\n", curr->tid, curr->name);
    } else {
        printf("ERROR: Failed to set/get current thread\n");
    }
    
    // Verify thread IDs are unique
    printf("\n=== Thread ID Uniqueness Test ===\n");
    if (t1->tid != t2->tid && t2->tid != t3->tid && t1->tid != t3->tid) {
        printf("SUCCESS: All thread IDs are unique\n");
        printf("  t1->tid = %d\n", t1->tid);
        printf("  t2->tid = %d\n", t2->tid);
        printf("  t3->tid = %d\n", t3->tid);
    } else {
        printf("ERROR: Thread IDs are not unique!\n");
    }
    
    printf("\n=== Thread Creation Test Complete ===\n");
}
