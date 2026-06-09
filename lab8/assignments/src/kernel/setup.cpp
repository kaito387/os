#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "memory.h"
#include "syscall.h"
#include "tss.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 内存管理器
MemoryManager memoryManager;
// 系统调用
SystemService systemService;
// Task State Segment
TSS tss;

int syscall_0(int first, int second, int third, int forth, int fifth)
{
    printf("systerm call 0: %d, %d, %d, %d, %d\n",
           first, second, third, forth, fifth);
    return first + second + third + forth + fifth;
}

// ==================== Assignment 1: 系统调用测试 ====================
// 测试自定义系统调用 factorial 和 get_pid
void a1_process()
{
    // 测试 get_pid 系统调用
    int pid = get_pid();
    printf("my pid: %d\n", pid);

    // 测试 factorial 系统调用
    int n = 5;
    int result = factorial(n);
    printf("factorial(%d) = %d\n", n, result);

    // 多次测试以验证正确性
    for (int i = 0; i <= 8; ++i) {
        printf("factorial(%d) = %d\n", i, factorial(i));
    }

    asm_halt();
}

// ==================== Assignment 2: Fork 测试 ====================
void a2_process()
{
    int pid = fork();
    int retval;

    if (pid)
    {
        pid = fork();
        if (pid)
        {
            while ((pid = wait(&retval)) != -1)
            {
                printf("wait for a child process, pid: %d, return value: %d\n",
                       pid, retval);
            }

            printf("all child process exit, programs: %d\n",
                   programManager.allPrograms.size());

            asm_halt();
        }
        else
        {
            uint32 tmp = 0xffffff;
            while (tmp)
                --tmp;
            printf("exit, pid: %d\n", programManager.running->pid);
            exit(123934);
        }
    }
    else
    {
        uint32 tmp = 0xffffff;
        while (tmp)
            --tmp;
        printf("exit, pid: %d\n", programManager.running->pid);
        exit(-123);
    }
}

// ==================== Assignment 3: Exit/Wait 与僵尸进程测试 ====================
// 场景：父进程先退出，子进程成为孤儿进程。测试僵尸进程回收机制。
void a3_process()
{
    int pid = fork();

    if (pid)
    {
        // 父进程
        printf("I am parent, pid: %d, my child pid: %d\n",
               get_pid(), pid);

        pid = fork();
        if (pid)
        {
            // 父进程不等待子进程，直接 exit
            // 这会使子进程成为孤儿进程
            printf("parent exit without waiting, pid: %d\n", get_pid());
            exit(0);
        }
        else
        {
            // 第二个子进程：延迟后退出
            uint32 tmp = 0xffffff;
            while (tmp)
                --tmp;
            printf("second child exit, pid: %d\n", get_pid());
            exit(456);
        }
    }
    else
    {
        // 第一个子进程：父进程退出后它成为孤儿进程
        // 延时等待父进程先退出（切换回父进程执行）
        uint32 tmp = 0xffffff;
        while (tmp)
            --tmp;
        // 此时父进程已经 exit 了
        printf("first child (orphan), pid: %d\n", get_pid());
        exit(123);
    }
}

// ==================== 选择执行哪个 Assignment ====================
// 修改此函数中的调用即可切换：a1_process / a2_process / a3_process
void second_thread(void *arg)
{
    printf("thread exit\n");
}

void first_thread(void *arg)
{
    printf("start process\n");

    // ★★★ 切换 Assignment：取消下面一行的注释即可 ★★★

    // programManager.executeProcess((const char *)a1_process, 1);   // Assignment 1
    programManager.executeProcess((const char *)a2_process, 1);   // Assignment 2
    // programManager.executeProcess((const char *)a3_process, 1);   // Assignment 3

    programManager.executeThread(second_thread, nullptr, "second", 1);
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

    // 初始化系统调用
    systemService.initialize();
    // 设置0号系统调用
    systemService.setSystemCall(0, (int)syscall_0);
    // 设置1号系统调用
    systemService.setSystemCall(1, (int)syscall_write);
    // 设置2号系统调用
    systemService.setSystemCall(2, (int)syscall_fork);
    // 设置3号系统调用
    systemService.setSystemCall(3, (int)syscall_exit);
    // 设置4号系统调用
    systemService.setSystemCall(4, (int)syscall_wait);
    // 设置6号系统调用：factorial — Assignment 1 自定义系统调用
    systemService.setSystemCall(6, (int)syscall_factorial);
    // 设置7号系统调用：get_pid — 辅助调试
    systemService.setSystemCall(7, (int)syscall_get_pid);

    // 内存管理器
    memoryManager.initialize();

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = ProgramStatus::RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
