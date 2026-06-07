#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "memory.h"
#include "dynamic_memory.h"
#include "page_replacement.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 内存管理器
MemoryManager memoryManager;

void test_dynamic_memory()
{
    printf("=== Assignment 2: Dynamic Partition Algorithm ===\n");

    // 分配4个页(16KB)作为动态内存管理区域
    char *heap = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 4);
    if (!heap)
    {
        printf("Failed to allocate heap for dynamic memory test\n");
        return;
    }
    printf("Heap base: 0x%x, size: 16384 bytes\n", (int)heap);

    DynamicMemory dynMem[2];  // 0=First-Fit, 1=Best-Fit

    // 初始化两个分配器，使用相同的初始内存布局
    // 各自使用2页(8KB)来管理
    dynMem[0].initialize(heap, 8192, FIRST_FIT);
    dynMem[1].initialize(heap + 8192, 8192, BEST_FIT);

    // 测试请求大小
    int sizes[] = {512, 256, 1024, 128, 768};
    int n = sizeof(sizes) / sizeof(sizes[0]);

    // 用两种策略分别分配
    void *ptrs[2][5];  // ptrs[0]=first-fit, ptrs[1]=best-fit

    for (int s = 0; s < 2; ++s)
    {
        printf("\n--- %s Allocation ---\n", dynMem[s].getStrategyName());
        for (int i = 0; i < n; ++i)
        {
            ptrs[s][i] = dynMem[s].allocate(sizes[i]);
            printf("Alloc %d bytes -> 0x%x\n", sizes[i], (int)ptrs[s][i]);
        }
        dynMem[s].showInfo();
    }

    // 释放部分内存，观察合并行为
    printf("\n--- Free some blocks and re-allocate ---\n");
    for (int s = 0; s < 2; ++s)
    {
        printf("\n[%s]\n", dynMem[s].getStrategyName());
        // 释放大小为256和128的块（在arr[1]和arr[3]的位置）
        dynMem[s].release(ptrs[s][1]);  // free 256
        dynMem[s].release(ptrs[s][3]);  // free 128
        printf("After free 256 and 128:\n");
        dynMem[s].showInfo();

        // 重新分配一个200字节的块
        void *p = dynMem[s].allocate(200);
        printf("Alloc 200 bytes -> 0x%x\n", (int)p);
        dynMem[s].showInfo();
    }

    printf("=== Dynamic Partition Test Complete ===\n\n");
}

void test_page_replacement()
{
    printf("=== Assignment 3: Page Replacement Algorithm ===\n");

    // 经典的页面引用序列（用于测试Belady异常等）
    // 使用3个物理帧，测试FIFO和LRU在相同引用序列下的表现
    unsigned int testSequence[] = {
        0xC1000000, 0xC1001000, 0xC1002000, 0xC1003000, 0xC1000000,
        0xC1001000, 0xC1004000, 0xC1000000, 0xC1001000, 0xC1002000,
        0xC1005000, 0xC1004000, 0xC1005000, 0xC1000000, 0xC1003000
    };
    int seqLen = sizeof(testSequence) / sizeof(testSequence[0]);
    int numFrames = 4;  // 4个物理帧

    printf("Reference sequence (%d pages):\n", seqLen);
    for (int i = 0; i < seqLen; ++i)
    {
        printf("  0x%x", testSequence[i]);
        if ((i + 1) % 5 == 0)
            printf("\n");
    }
    printf("\n\n");

    // 测试FIFO
    printf("--- Testing FIFO with %d frames ---\n", numFrames);
    PageReplacementManager prFIFO;
    prFIFO.initialize(numFrames, REPLACE_FIFO);
    for (int i = 0; i < seqLen; ++i)
    {
        int result = prFIFO.accessPage(testSequence[i]);
        if (!result)
        {
            printf("FIFO: failed to access page 0x%x\n", testSequence[i]);
        }
    }
    prFIFO.showStatus();

    // 测试LRU
    printf("\n--- Testing LRU with %d frames ---\n", numFrames);
    PageReplacementManager prLRU;
    prLRU.initialize(numFrames, REPLACE_LRU);
    for (int i = 0; i < seqLen; ++i)
    {
        int result = prLRU.accessPage(testSequence[i]);
        if (!result)
        {
            printf("LRU: failed to access page 0x%x\n", testSequence[i]);
        }
    }
    prLRU.showStatus();

    // 对比不同帧数对FIFO的影响（Belady异常演示）
    printf("\n--- Belady's Anomaly Test (FIFO) ---\n");
    unsigned int beladySeq[] = {
        0xC2000000, 0xC2001000, 0xC2002000, 0xC2003000, 0xC2000000,
        0xC2004000, 0xC2000000, 0xC2001000, 0xC2002000, 0xC2003000,
        0xC2004000, 0xC2005000
    };
    int beladyLen = sizeof(beladySeq) / sizeof(beladySeq[0]);

    for (int f = 3; f <= 4; ++f)
    {
        printf("FIFO with %d frames:\n", f);
        PageReplacementManager pr;
        pr.initialize(f, REPLACE_FIFO);
        for (int i = 0; i < beladyLen; ++i)
        {
            pr.accessPage(beladySeq[i]);
        }
        printf("  Page faults: %d / %d (rate: %d%%)\n",
               pr.getPageFaultCount(), pr.getPageAccessCount(),
               pr.getPageFaultCount() * 100 / pr.getPageAccessCount());
    }

    printf("=== Page Replacement Test Complete ===\n\n");
}

// ===== Assignment 4 深度测试: 虚拟页内存管理 bug 分析 =====

// 测试1: 单页分配与数据读写验证
// 目的: 验证分配的单页确实可读写，映射关系正确
int test_vm_single_page_rw()
{
    printf("\n[Test 1] Single page allocate and write/read\n");
    int passed = 1;

    char *p = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
    if (!p)
    {
        printf("  FAIL: cannot allocate 1 page\n");
        return 0;
    }
    printf("  Allocated 1 page at 0x%x\n", (int)p);

    // 在页内多处写入特征值
    p[0] = 0x41;
    p[1024] = 0x42;
    p[2048] = 0x43;
    p[4095] = 0x44;

    // 读回验证
    if (p[0] != 0x41 || p[1024] != 0x42 ||
        p[2048] != 0x43 || p[4095] != 0x44)
    {
        printf("  FAIL: data readback mismatch: %x %x %x %x\n",
               p[0], p[1024], p[2048], p[4095]);
        passed = 0;
    }
    else
    {
        printf("  PASS: data readback correct\n");
    }

    memoryManager.releasePages(AddressPoolType::KERNEL, (int)p, 1);
    return passed;
}

// 测试2: 跨4MB边界分配（触发新页表的创建）
// 目的: 验证当分配跨过4MB边界时，能自动创建新的页表
int test_vm_cross_pde_boundary()
{
    printf("\n[Test 2] Cross 4MB boundary (new page table creation)\n");
    int passed = 1;

    // 内核虚拟地址池从 0xC0100000 开始
    // PDE index 768 覆盖 0xC0000000~0xC03FFFFF
    // PDE index 769 覆盖 0xC0400000~0xC07FFFFF
    // 分配足够多的页来跨过 0xC0400000 边界
    // 从 0xC0100000 到 0xC0400000 需要 (0x300000 / 0x1000) = 768 页

    int pagesNeeded = 770;  // 比768多一点，确保跨过边界
    char *p = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, pagesNeeded);
    if (!p)
    {
        printf("  FAIL: cannot allocate %d pages\n", pagesNeeded);
        return 0;
    }
    printf("  Allocated %d pages from 0x%x to 0x%x\n",
           pagesNeeded, (int)p, (int)(p + pagesNeeded * PAGE_SIZE));

    // 在第一个页和最后一个页写入数据验证映射
    p[0] = 0x51;
    p[pagesNeeded * PAGE_SIZE - 1] = 0x52;

    if (p[0] != 0x51)
    {
        printf("  FAIL: first page readback failed\n");
        passed = 0;
    }
    if (p[pagesNeeded * PAGE_SIZE - 1] != 0x52)
    {
        printf("  FAIL: last page readback failed\n");
        passed = 0;
    }

    if (passed)
        printf("  PASS: cross-boundary access correct\n");

    memoryManager.releasePages(AddressPoolType::KERNEL, (int)p, pagesNeeded);
    return passed;
}

// 测试3: 碎片回收与复用
// 目的: 验证释放后空间能被正确复用
int test_vm_fragment_reuse()
{
    printf("\n[Test 3] Fragment recycle and reuse\n");
    int passed = 1;

    // 分配三段连续空间
    char *a = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 50);
    char *b = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 50);
    char *c = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 50);

    if (!a || !b || !c)
    {
        printf("  FAIL: cannot allocate blocks\n");
        return 0;
    }
    printf("  Allocated: a=0x%x, b=0x%x, c=0x%x\n", (int)a, (int)b, (int)c);

    // 验证连续性
    if (b != a + 50 * PAGE_SIZE || c != b + 50 * PAGE_SIZE)
    {
        printf("  FAIL: blocks not contiguous as expected\n");
        passed = 0;
    }

    // 在每段写特征值
    a[0] = 0xAA;
    b[0] = 0xBB;
    c[0] = 0xCC;

    // 释放中间段
    memoryManager.releasePages(AddressPoolType::KERNEL, (int)b, 50);
    printf("  Released b (50 pages)\n");

    // 分配20页——应该复用b的位置
    char *d = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 20);
    printf("  Allocated 20 pages at 0x%x (expected 0x%x)\n", (int)d, (int)b);

    if (d != b)
    {
        printf("  FAIL: did not reuse freed space (got 0x%x, expected 0x%x)\n", (int)d, (int)b);
        passed = 0;
    }
    else
    {
        printf("  PASS: correctly reused freed space\n");
        d[0] = 0xDD;
    }

    // 再分配20页（应该紧随d之后，也在b的原空间内）
    char *e = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 20);
    printf("  Allocated 20 pages at 0x%x (expected 0x%x)\n", (int)e, (int)(d + 20 * PAGE_SIZE));

    if (e != d + 20 * PAGE_SIZE)
    {
        printf("  FAIL: second reuse position wrong\n");
        passed = 0;
    }
    else
    {
        printf("  PASS: second reuse position correct\n");
    }

    // 清理
    memoryManager.releasePages(AddressPoolType::KERNEL, (int)a, 50);
    memoryManager.releasePages(AddressPoolType::KERNEL, (int)d, 20);
    memoryManager.releasePages(AddressPoolType::KERNEL, (int)e, 20);
    memoryManager.releasePages(AddressPoolType::KERNEL, (int)c, 50);

    return passed;
}

// 测试4: 分配0页——边界条件
int test_vm_zero_pages()
{
    printf("\n[Test 4] Allocate 0 pages (edge case)\n");

    char *p = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 0);
    if (p != 0)
    {
        printf("  FAIL: allocate 0 pages should return 0, got 0x%x\n", (int)p);
        return 0;
    }
    printf("  PASS: allocate 0 pages correctly returns 0\n");
    return 1;
}

// 测试5: 大量分配释放循环（压力测试）
int test_vm_alloc_free_loop()
{
    printf("\n[Test 5] Allocate/free stress test\n");
    int passed = 1;

    // 分配100个单独的页，记录地址
    const int N = 100;
    char *ptrs[N];

    for (int i = 0; i < N; ++i)
    {
        ptrs[i] = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
        if (!ptrs[i])
        {
            printf("  FAIL: allocate page %d failed\n", i);
            return 0;
        }
        ptrs[i][0] = (char)(i & 0xff);  // 写特征值
    }
    printf("  Allocated %d single pages\n", N);

    // 验证每个页的数据完整
    int corrupt = 0;
    for (int i = 0; i < N; ++i)
    {
        if (ptrs[i][0] != (char)(i & 0xff))
        {
            printf("  FAIL: page %d corrupted (expected %x, got %x)\n",
                   i, (char)(i & 0xff), ptrs[i][0]);
            corrupt++;
            passed = 0;
        }
    }
    if (!corrupt)
        printf("  PASS: all %d pages intact after mass allocation\n", N);

    // 释放所有页
    for (int i = 0; i < N; ++i)
    {
        memoryManager.releasePages(AddressPoolType::KERNEL, (int)ptrs[i], 1);
    }
    printf("  Released all %d pages\n", N);

    // 再次分配，确保空间可复用
    char *big = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, N);
    if (!big)
    {
        printf("  FAIL: cannot re-allocate %d pages after freeing\n", N);
        passed = 0;
    }
    else
    {
        printf("  PASS: re-allocated %d contiguous pages after freeing\n", N);
        memoryManager.releasePages(AddressPoolType::KERNEL, (int)big, N);
    }

    return passed;
}

// 测试6: vaddr2paddr 正确性验证
int test_vm_vaddr2paddr()
{
    printf("\n[Test 6] vaddr2paddr correctness\n");
    int passed = 1;

    // 分配一页
    char *p = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
    if (!p)
    {
        printf("  FAIL: cannot allocate page\n");
        return 0;
    }

    // 写入数据
    p[0] = 0x77;
    p[100] = 0x88;

    // 通过 vaddr2paddr 获取物理地址
    int phys = memoryManager.vaddr2paddr((int)p);
    printf("  Virtual 0x%x -> Physical 0x%x\n", (int)p, phys);

    // 物理地址应该 > 0（有效映射）
    if (phys == 0)
    {
        printf("  FAIL: vaddr2paddr returned 0\n");
        passed = 0;
    }

    // 页内偏移应该保留
    int phys2 = memoryManager.vaddr2paddr((int)(p + 100));
    if (phys2 != phys + 100)
    {
        printf("  FAIL: vaddr2paddr offset broken (expected 0x%x, got 0x%x)\n",
               phys + 100, phys2);
        passed = 0;
    }
    else
    {
        printf("  PASS: vaddr2paddr offset correct\n");
    }

    memoryManager.releasePages(AddressPoolType::KERNEL, (int)p, 1);
    return passed;
}

// 综合测试入口
void test_vm_deep()
{
    printf("========== Assignment 4 Deep Test: Bug Analysis ==========\n");

    int t1 = test_vm_single_page_rw();
    int t2 = test_vm_cross_pde_boundary();
    int t3 = test_vm_fragment_reuse();
    int t4 = test_vm_zero_pages();
    int t5 = test_vm_alloc_free_loop();
    int t6 = test_vm_vaddr2paddr();

    printf("\n========== Results: %d/6 tests passed ==========\n\n",
           t1 + t2 + t3 + t4 + t5 + t6);
}

void first_thread(void *arg)
{
    // 第1个线程不可以返回

    // 测试 Assignment 1 & 4: 虚拟页内存管理（基础测试）
    printf("=== Assignment 1 & 4: Basic Virtual Page Test ===\n");
    char *p1 = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 100);
    char *p2 = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 10);
    char *p3 = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 100);

    printf("%x %x %x\n", p1, p2, p3);

    memoryManager.releasePages(AddressPoolType::KERNEL, (int)p2, 10);
    p2 = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 100);

    printf("%x\n", p2);

    p2 = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 10);

    printf("%x\n", p2);
    printf("=== Basic Test Complete ===\n\n");

    // Assignment 4 深度 bug 分析测试
    test_vm_deep();

    // 测试 Assignment 2: 动态分区算法
    test_dynamic_memory();

    // 测试 Assignment 3: 页面置换算法
    test_page_replacement();

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

    // 内存管理器
    memoryManager.openPageMechanism();
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
    firstThread->status = RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
