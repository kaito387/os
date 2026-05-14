#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;

const int N = 5, DELAY = 0x3fffff;

Semaphore chopsticks[N];

#define LEFT (who_am_i + N - 1) % N
#define RIGHT (who_am_i + 1) % N

void mysleep(int time) {
    for (int i = 0; i < time; ++i) {
        asm volatile("nop");
    }
}
void THINK() {
    mysleep(DELAY);
}
void EAT() {
    mysleep(DELAY);
}

void philosopher(void *arg) {
    int who_am_i = (int)arg;
    while (true) {
        THINK();
        chopsticks[LEFT].P();
        chopsticks[RIGHT].P();
        EAT();
        printf("philosopher %d eat\n", who_am_i);
        chopsticks[LEFT].V();
        chopsticks[RIGHT].V();
    }
}

void first_thread(void *arg)
{
    // 第1个线程不可以返回
    stdio.moveCursor(0);
    for (int i = 0; i < 25 * 80; ++i)
    {
        stdio.print(' ');
    }
    stdio.moveCursor(0);

    for (int i = 0; i < N; ++i) {
        chopsticks[i].initialize(1);
        programManager.executeThread(philosopher, (void *)i, "philosopher", 1);
    }

    asm_halt();
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
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
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
    asm_switch_thread(0, firstThread);

    asm_halt();
}
