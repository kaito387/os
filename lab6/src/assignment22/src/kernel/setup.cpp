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

Semaphore empty, full;

int dish;
const int MAX_DISHES = 5, DELAY = 0x1ffffff;

void mysleep(int delay) {
    while (delay--) {
        // do nothing
    }
}

void chef_1(void *arg) {
    while (1) {
        empty.P();
        printf("chef 1: There are only %d dishes.\n", dish);
        mysleep(DELAY); // making a dish
        ++dish;
        full.V();
        printf("chef 1: I have made a dish!\n");
    }
}
void chef_2(void *arg) {
    while (1) {
        empty.P();
        printf("chef 2: There are only %d dishes.\n", dish);
        mysleep(DELAY); // making a dish
        ++dish;
        full.V();
        printf("chef 2: I have made a dish!\n");
    }
}

void a_customer(void *arg) {
    mysleep(10 * DELAY); // coming to the restaurant
    if (dish > MAX_DISHES) {
        printf("customer: WTF there are %d dishes but only %d plates. I'll eat them up.\n", dish, MAX_DISHES);
    } else {
        printf("customer: There are only %d dishes. I'll eat them up.\n", dish);
    }
    while (1) {
        full.P();
        --dish;
        empty.V();
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

    dish = 0;
    empty.initialize(MAX_DISHES);
    full.initialize(0);

    programManager.executeThread(chef_1, nullptr, "first chef", 1);
    programManager.executeThread(chef_2, nullptr, "second chef", 1);
    programManager.executeThread(a_customer, nullptr, "customer", 1);

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
