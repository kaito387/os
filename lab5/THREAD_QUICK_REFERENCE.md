# Lab5 Thread Creation - Quick Reference Guide

## API Reference

### Initialize Thread System
```c
void thread_init(void)
```
Initializes thread and stack pools. Must be called once before creating threads.

### Create a Thread
```c
thread_t* thread_create(ThreadFunction entry, void *arg, const char *name, uint32 priority)
```
- **entry**: Function pointer to thread's main code
- **arg**: Single argument to pass to entry function (can be cast to any type)
- **name**: Human-readable name (max 16 chars)
- **priority**: 1-based priority (higher = more CPU time)
- **returns**: Pointer to thread_t on success, NULL on failure

Example:
```c
void my_thread(void *arg) {
    int id = (int)arg;
    printf("Thread %d running\n", id);
}

thread_t *t = thread_create(my_thread, (void *)1, "worker", 10);
```

### Get/Set Current Thread
```c
thread_t* thread_current(void)           // Get currently running thread
void thread_set_current(thread_t *thread)  // Set current thread
```

### Cleanup Thread
```c
void thread_destroy(thread_t *thread)
```
Frees thread structure and stack back to pools.

## Thread Structure Members

```c
struct thread_t {
    uint32 *stack;        // Stack pointer for context switch
    char name[17];        // Thread name
    ThreadState state;    // READY, RUNNING, BLOCKED, DEAD
    uint32 priority;      // Priority (1-10+)
    uint32 tid;           // Unique thread ID
    uint32 ticks;         // Time quantum = priority * 10
    uint32 ticks_passed;  // Used ticks this quantum
    
    // x86 registers
    uint32 eax, ebx, ecx, edx, esi, edi, esp, ebp, eip;
};
```

## Thread States
- **READY (0)**: Waiting to run, in ready queue
- **RUNNING (1)**: Currently executing
- **BLOCKED (2)**: Waiting for I/O or event
- **DEAD (3)**: Finished, ready for cleanup

## Memory Model

```
Thread Pool (40 KB total):
├─ PCB[0] (4KB)
├─ PCB[1] (4KB)
├─ ...
└─ PCB[9] (4KB)

Stack Pool (40 KB total):
├─ Stack[0] (4KB, grows downward)
├─ Stack[1] (4KB, grows downward)
├─ ...
└─ Stack[9] (4KB, grows downward)
```

## Typical Usage

```c
// Main setup
void kernel_setup() {
    thread_init();
    
    // Create threads
    thread_t *t1 = thread_create(worker1, (void *)1, "worker1", 10);
    thread_t *t2 = thread_create(worker2, (void *)2, "worker2", 8);
    thread_t *t3 = thread_create(worker3, (void *)3, "worker3", 6);
    
    if (!t1 || !t2 || !t3) {
        printf("ERROR: Thread creation failed\n");
        return;
    }
    
    // Schedule threads (in future assignment)
    scheduler_run();
}

// Thread function
void worker1(void *arg) {
    printf("Worker 1 ID: %d\n", (int)arg);
    // Do work...
    // Thread automatically calls thread_exit when returns
}
```

## Limitations
- Maximum 10 threads
- No automatic thread switching (scheduler not yet implemented)
- All threads share errno and global variables
- No thread-local storage (TLS) yet
- No inter-thread synchronization primitives

## Compilation
```bash
# Build thread module
make thread-test

# Build everything
make compile-all

# Run tests
# (Not yet executable in kernel environment, compile to object only)
```

## Debugging Tips
1. Check thread IDs are unique (should be 1, 2, 3, ...)
2. Verify stack pointers don't overlap
3. Ensure thread names are properly null-terminated
4. Priority should be > 0 (recommended 1-10)
5. Use printf() in thread functions to trace execution

## Common Issues

### Q: Thread creation returns NULL
- **A**: Thread or stack pool is full (max 10 threads)

### Q: Stack corruption
- **A**: Check stack doesn't overflow (each thread gets 4KB)

### Q: Thread doesn't execute
- **A**: Scheduler not yet implemented; use thread_set_current() to manually run

### Q: Can't get current thread
- **A**: Must call thread_set_current() first

## Next Steps
1. Implement context switching in assembly
2. Implement scheduler (round-robin)
3. Implement synchronization primitives
4. Add inter-thread communication
5. Extend with dynamic thread creation
