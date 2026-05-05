#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;

void third_thread(void *arg) {
    printf("Working on thread 3 (pid %d, priority %d)\n", programManager.running->pid, programManager.running->priority);
    program_exit();
}
void forth_thread(void *arg) {
    printf("Working on thread 4 (pid %d, priority %d)\n", programManager.running->pid, programManager.running->priority);
    program_exit();
}

void second_thread(void *arg) {
    printf("Working on thread 2 (pid %d, priority %d)\n", programManager.running->pid, programManager.running->priority);
    programManager.executeThread(third_thread, nullptr, "third_thread", 4);
    programManager.executeThread(forth_thread, nullptr, "forth_thread", 2);
    printf("Second thread completed\n");
    program_exit();
}

void first_thread(void *arg)
{
    printf("Working on thread 1 (pid %d, priority %d)\n", programManager.running->pid, programManager.running->priority);
    programManager.executeThread(second_thread, nullptr, "second_thread", 3);
    printf("First thread completed\n");
    program_exit();
}

extern "C" void setup_kernel()
{   
    // 中断管理器
    interruptManager.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);
    
    // 输出管理器
    stdio.initialize();
    printf("STDIO initialized. Addr: %x\n", (uint32)&stdio);

    // 进程/线程管理器
    programManager.initialize();
    printf("ProgramManager initialized. Addr: %x\n", (uint32)&programManager);

    programManager.executeThread(first_thread, nullptr, "first_thread", 1);
    
    interruptManager.enableInterrupt();
    while (programManager.readyPrograms.size() > 0)
    {
        printf("Start scheduling\n");
        programManager.schedulePriority();
    }

    // 主调度循环 - 不应该到达这里，但作为安全措施
    printf("scheduling ended\n");
    asm_halt();
}
