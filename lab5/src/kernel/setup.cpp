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
    for(int i = 0; i < 3; i++) {
        printf("Thread 3 (pid %d): iteration %d\n", programManager.running->pid, i);
    }
    program_exit();
}

void second_thread(void *arg) {
    for(int i = 0; i < 3; i++) {
        printf("Thread 2 (pid %d): iteration %d\n", programManager.running->pid, i);
    }
    program_exit();
}

void first_thread(void *arg)
{
    // 第1个线程创建其他线程
    printf("Thread 1 (pid %d): starting\n", programManager.running->pid);
    if (!programManager.running->pid)
    {
        programManager.executeThread(second_thread, nullptr, "second_thread", 1);
        programManager.executeThread(third_thread, nullptr, "third_thread", 1);
    }
    for(int i = 0; i < 3; i++) {
        printf("Thread 1 (pid %d): iteration %d\n", programManager.running->pid, i);
    }
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

    // 进程/线程管理器
    programManager.initialize();

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first_thread", 1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    
    // 启用中断并切换到第一个线程
    interruptManager.enableInterrupt();
    asm_switch_thread(0, firstThread);

    // 主调度循环 - 不应该到达这里，但作为安全措施
    asm_halt();
}
