# Assignment 4: 调度算法的实现

## 目标

在基础的时间片轮转 (Round-Robin) 调度的基础上，实现并比较多种线程调度算法。

## 已实现的调度算法

### 1. 时间片轮转 (Round-Robin, RR) - 默认算法

**位置：** [src/kernel/program.cpp](../src/kernel/program.cpp#L74-L110) - `schedule()` 方法

**原理：**
- 所有就绪线程共享 CPU 时间
- 每个线程获得等量的 CPU 时间片
- 当时间片用完时，线程放回队列末尾
- 调度器总是选择队列首部的线程

**实现代码：**
```cpp
void ProgramManager::schedule()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }

    // 当前线程时间片用完，放回队列
    if (running->status == ProgramStatus::RUNNING)
    {
        running->status = ProgramStatus::READY;
        running->ticks = running->priority * 10;  // 重置时间片
        readyPrograms.push_back(&(running->tagInGeneralList));  // 放回队尾
    }
    else if (running->status == ProgramStatus::DEAD)
    {
        releasePCB(running);  // 释放已死亡线程的 PCB
    }

    // 从队列首部选择下一个线程
    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item, tagInGeneralList);
    PCB *cur = running;
    
    next->status = ProgramStatus::RUNNING;
    running = next;
    readyPrograms.pop_front();  // 从队列移除

    asm_switch_thread(cur, next);  // 执行上下文切换

    interruptManager.setInterruptStatus(status);
}
```

**特点：**
- ✓ 公平性好 - 所有线程机会均等
- ✓ 响应时间均衡
- ✓ 实现简单
- ✗ 可能不适合实时任务
- ✗ I/O 密集型任务性能较差

**应用场景：**
- 通用操作系统
- 桌面系统
- 时间敏感性不强的环境

**时间复杂度：**
- 调度时间：O(1)
- 队列操作：O(1)

---

### 2. 优先级调度 - 新增算法

**位置：** [src/kernel/program.cpp](../src/kernel/program.cpp#L114-L160) - `schedulePriority()` 方法

**原理：**
- 每个线程有优先级属性（0-最低，N-最高）
- 调度器总是选择优先级最高的就绪线程
- 高优先级线程会优先获得 CPU 时间
- 低优先级线程可能被"饿死"

**实现代码：**
```cpp
void ProgramManager::schedulePriority()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }

    // 处理当前线程
    if (running->status == ProgramStatus::RUNNING)
    {
        running->status = ProgramStatus::READY;
        readyPrograms.push_back(&(running->tagInGeneralList));
    }
    else if (running->status == ProgramStatus::DEAD)
    {
        releasePCB(running);
    }

    // 遍历就绪队列，找优先级最高的线程
    ListItem *item = readyPrograms.front();
    PCB *highest = ListItem2PCB(item, tagInGeneralList);
    ListItem *current = item->next;
    
    while (current != &(readyPrograms.head))
    {
        PCB *thread = ListItem2PCB(current, tagInGeneralList);
        if (thread->priority > highest->priority)  // 比较优先级
        {
            highest = thread;
            item = current;
        }
        current = current->next;
    }

    // 从队列中移除选中的线程
    readyPrograms.erase(item);
    
    PCB *next = highest;
    PCB *cur = running;
    
    next->status = ProgramStatus::RUNNING;
    running = next;

    asm_switch_thread(cur, next);

    interruptManager.setInterruptStatus(status);
}
```

**特点：**
- ✓ 高优先级任务响应快
- ✓ 适合实时系统
- ✗ 公平性差 - 低优先级可能饿死
- ✗ 实现复杂度中等
- ✗ 需要小心设置优先级

**应用场景：**
- 实时操作系统 (RTOS)
- 医疗设备
- 工业控制
- 优先级明确的系统

**时间复杂度：**
- 调度时间：O(n) - 需要遍历队列找最高优先级
- 队列操作：O(1)

---

## 切换调度算法

### 方法 1: 修改中断处理程序

编辑 [src/kernel/interrupt.cpp](../src/kernel/interrupt.cpp#L88)：

```cpp
extern "C" void c_time_interrupt_handler()
{
    PCB *cur = programManager.running;

    if (cur->ticks)
    {
        --cur->ticks;
        ++cur->ticksPassedBy;
    }
    else
    {
        // 切换这一行来改变调度算法：
        // programManager.schedule();           // 时间片轮转
        programManager.schedulePriority();      // 优先级调度
    }
}
```

### 方法 2: 使用条件编译

```cpp
// 在 interrupt.cpp 顶部添加：
#define USE_PRIORITY_SCHEDULING 0  // 1 = 优先级调度, 0 = 时间片轮转

extern "C" void c_time_interrupt_handler()
{
    PCB *cur = programManager.running;

    if (cur->ticks)
    {
        --cur->ticks;
        ++cur->ticksPassedBy;
    }
    else
    {
        #if USE_PRIORITY_SCHEDULING
            programManager.schedulePriority();
        #else
            programManager.schedule();
        #endif
    }
}
```

然后在 Makefile 中添加：
```makefile
CXXFLAGS += -DUSE_PRIORITY_SCHEDULING=1
```

---

## 测试不同的调度算法

### 测试程序设置

当前 [setup.cpp](../src/kernel/setup.cpp) 创建三个优先级相同的线程，每个线程执行 3 次迭代：

```cpp
programManager.executeThread(second_thread, nullptr, "second_thread", 1);
programManager.executeThread(third_thread, nullptr, "third_thread", 1);
```

### 扩展测试：不同优先级

要测试优先级调度的效果，可以修改优先级：

```cpp
// 优先级不同的线程创建
programManager.executeThread(second_thread, nullptr, "second_thread", 3);  // 高优先级
programManager.executeThread(third_thread, nullptr, "third_thread", 1);   // 低优先级
```

**预期行为：**
- 使用 RR 调度时：三个线程均匀轮转
- 使用优先级调度时：`second_thread` 会被更频繁地执行

### 性能指标

可以添加计数器来测量：

```cpp
struct PCB
{
    // ... 现有字段 ...
    int executionCount;    // 被执行的次数
    int totalTimeUsed;     // 总共使用的时间
};
```

然后在中断处理中更新：
```cpp
programManager.running->executionCount++;
programManager.running->totalTimeUsed++;
```

---

## 其他可能实现的调度算法

### 1. 先来先服务 (First-Come-First-Served, FCFS)

最简单的算法 - 线程按创建顺序执行，直到完成不换出。

```cpp
void ProgramManager::scheduleFCFS()
{
    // 仅当当前线程完成时才切换到下一个
    if (running->status == ProgramStatus::DEAD || readyPrograms.size() > 0)
    {
        // 选择就绪队列第一个...
    }
}
```

### 2. 最短作业优先 (Shortest Job First, SJF)

选择预计执行时间最短的线程。

```cpp
void ProgramManager::scheduleSJF()
{
    // 遍历队列，找执行时间最短的
    // 这需要在 PCB 中添加预计执行时间字段
}
```

### 3. 多级反馈队列 (Multilevel Feedback Queue)

结合 RR 和优先级的优势：
- 多个就绪队列，不同优先级
- 同一队列内使用 RR
- 不同队列间使用优先级

### 4. 完全公平调度器 (Completely Fair Scheduler, CFS)

Linux 使用的算法，使用红黑树实现：
- 跟踪每个线程的"虚拟运行时间"
- 始终选择虚拟运行时间最短的线程
- 提供高度的公平性和性能

---

## 性能比较

### 场景 1: 等优先级线程

| 算法 | CPU 利用率 | 响应时间 | 公平性 |
|------|-----------|--------|-------|
| RR   | 高        | 均衡    | 高    |
| 优先级 | 高       | 不均衡  | 低    |
| FCFS | 中        | 不均衡  | 低    |

### 场景 2: 不同优先级线程

| 算法 | 高优先级响应 | 低优先级响应 | 饥荒风险 |
|------|-----------|-----------|--------|
| RR   | 慢         | 慢         | 无     |
| 优先级 | 快        | 极慢      | 高     |
| 多级队列 | 快     | 中等       | 低     |

---

## 添加新调度算法

要添加新的调度算法，需要：

1. **在 `program.h` 中声明新方法：**
```cpp
class ProgramManager
{
    // ...
    void scheduleNewAlgorithm();
};
```

2. **在 `program.cpp` 中实现：**
```cpp
void ProgramManager::scheduleNewAlgorithm()
{
    // 实现算法逻辑
}
```

3. **在 `interrupt.cpp` 中调用：**
```cpp
extern "C" void c_time_interrupt_handler()
{
    // ...
    programManager.scheduleNewAlgorithm();
}
```

4. **编译测试：**
```bash
cd build
make clean
make build
make run
```

---

## 调试和验证

### 打印调度信息

在 `schedule()` 方法中添加调试输出：

```cpp
printf("[SCHEDULE] Thread %s (PID %d) -> %s (PID %d)\n", 
       cur->name, cur->pid, next->name, next->pid);
```

### 验证调度顺序

创建测试程序，记录线程执行顺序：

```cpp
int execution_order[100];
int order_index = 0;

void test_thread(void *arg)
{
    execution_order[order_index++] = programManager.running->pid;
    // ...
}
```

### 使用 GDB 跟踪

```bash
(gdb) break schedule
(gdb) commands
> print programManager.running->name
> print next->name
> continue
> end
```

---

## 性能优化建议

### 1. 优先级调度的优化

当前实现是 O(n)，可以优化为 O(log n)：
- 使用堆 (Heap) 维护就绪队列
- 或使用红黑树

### 2. 缓存友好

- 保持线程局部性
- 减少上下文切换的缓存污染

### 3. 动态优先级调整

- 根据线程行为调整优先级
- 防止优先级反演

---

## 参考资源

1. **操作系统教材：**
   - 《现代操作系统》- Andrew S. Tanenbaum
   - 《操作系统概念》- Abraham Silberschatz

2. **实现参考：**
   - Linux 内核调度器源码
   - xv6 教学操作系统
   - MicroKernel 的调度实现

3. **在线资源：**
   - CPU Scheduling 算法演示
   - OS 调度可视化工具

---

## 总结

- ✓ 时间片轮转提供了公平的 CPU 分配
- ✓ 优先级调度适合实时系统
- ✓ 易于扩展新的调度算法
- ✓ 可以通过修改中断处理切换算法
- ✓ 需要平衡公平性、响应时间和实现复杂度
