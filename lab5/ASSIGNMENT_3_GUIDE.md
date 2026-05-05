# Assignment 3: 线程调度切换的秘密 - 实践演示

## 目标

通过使用 GDB 调试器跟踪内核线程的切换过程，理解操作系统是如何实现并发执行的。

## 核心原理

线程并发执行的秘密在于**时间片轮转**：
1. **中断** - 时钟中断打断当前线程
2. **保存状态** - CPU 自动保存当前执行状态
3. **调度决策** - 内核调度器选择下一个线程
4. **上下文切换** - 切换到新线程的执行栈和寄存器
5. **恢复执行** - 新线程从被中断处继续执行

## 准备工作

### 1. 编译和启动

```bash
cd /root/dev/study/os/lab5/build

# 清理旧文件
make clean

# 编译内核
make build

# 启动 QEMU 调试模式
make debug
```

这会启动两个窗口：
- 第一个窗口：QEMU 虚拟机 (等待 GDB 连接)
- 第二个窗口：GDB 调试器

### 2. GDB 基本命令回顾

```bash
# 启动 GDB
gdb

# 连接到本地 QEMU 调试服务器
(gdb) target remote localhost:1234

# 设置断点
(gdb) break functionName
(gdb) break fileName.cpp:lineNumber

# 控制执行
(gdb) continue (c)      - 继续执行直到下一个断点
(gdb) next (n)          - 单步执行（不进入函数）
(gdb) step (s)          - 单步执行（进入函数）
(gdb) ni                - 汇编单步（不进入函数）
(gdb) si                - 汇编单步（进入函数）

# 查看信息
(gdb) info registers    - 显示所有寄存器
(gdb) info locals       - 显示本地变量
(gdb) print variable    - 打印变量值 (p 是简写)
(gdb) backtrace (bt)    - 显示函数调用栈
(gdb) disassemble       - 反汇编当前函数
```

## 演示 1: 观察新线程的创建和启动

### 步骤

1. **在 GDB 中设置断点**
```bash
(gdb) break executeThread
(gdb) break asm_switch_thread
(gdb) continue
```

2. **观察线程创建过程**

当断点在 `executeThread` 时：
```bash
(gdb) p programManager.running
$1 = (PCB *) 0x...

(gdb) print threadName
$2 = "first_thread"

(gdb) print priority
$3 = 1
```

继续执行，观察栈的初始化：
```bash
(gdb) step
...
(gdb) p thread->stack[0]  # 返回地址
$4 = 0x...
(gdb) p thread->stack[4]  # 函数指针
$5 = 0x...
```

3. **观察第一次线程切换**

当代码执行到 `asm_switch_thread` 时：
```bash
(gdb) info registers esp ebp eip
esp            0x...        0x...
ebp            0x...        0x...
eip            0x...        0x...

# 进入汇编代码
(gdb) si
(gdb) disassemble
   ...
=> 0x... <asm_switch_thread+0>:    push   %ebp
   0x... <asm_switch_thread+1>:    mov    %esp,%ebp
   ...
```

4. **观察 ESP 的变化**

```bash
# 保存当前 ESP
(gdb) p $esp
$6 = 0x8000000

# 切换后的 ESP（从第一个线程的栈）
(gdb) p firstThread->stack
$7 = (int *) 0x8001000

# 执行切换指令
(gdb) si
(gdb) si
(gdb) si

# 观察 ESP 已改变
(gdb) p $esp
$8 = 0x8001000  # 已切换到新线程的栈
```

## 演示 2: 观察时间中断处理和线程切换

### 步骤

1. **在时间中断处理处设置断点**
```bash
(gdb) break c_time_interrupt_handler
(gdb) continue
```

内核运行一段时间后，当时间中断发生时会停止。

2. **观察中断处理流程**
```bash
# 查看调用栈
(gdb) backtrace
#0  c_time_interrupt_handler () at interrupt.cpp:...
#1  <signal handler called>
#2  0x... in first_thread (arg=0x...) at setup.cpp:...
#3  ...

# 观察当前线程
(gdb) p programManager.running->name
$9 = "first_thread"

(gdb) p programManager.running->ticks
$10 = 0  # 时间片已用完
```

3. **进入调度函数**
```bash
(gdb) step
# 进入 schedule() 函数

# 观察就绪队列
(gdb) p programManager.readyPrograms.size()
$11 = 3  # 有三个线程在就绪队列中

# 观察被选中的下一个线程
(gdb) p next->name
$12 = "second_thread"

(gdb) p next->pid
$13 = 1
```

4. **观察线程切换前后**

切换前：
```bash
(gdb) p programManager.running->name
$14 = "first_thread"
(gdb) p $esp
$15 = 0x1000000  # 第一个线程的栈
```

进入 `asm_switch_thread`：
```bash
(gdb) step
# 现在在汇编代码中

# 查看 EBP, ESP 和其他寄存器
(gdb) info registers
eax            0x...
ebx            0x...
esp            0x...
...

# 单步执行汇编指令
(gdb) si
(gdb) si
(gdb) si
(gdb) si
```

切换后：
```bash
(gdb) finish  # 返回到 schedule() 函数
(gdb) p programManager.running->name
$16 = "second_thread"  # 已切换！

(gdb) p $esp
$17 = 0x2000000  # 现在是第二个线程的栈
```

## 演示 3: 连续观察多次线程切换

### 步骤

```bash
# 设置自动断点
(gdb) break c_time_interrupt_handler

# 创建命令序列 (按 Enter 时自动执行)
(gdb) commands
> printf "Interrupt! Current thread: %s (PID %d)\n", programManager.running->name, programManager.running->pid
> printf "Ready queue size: %d\n", programManager.readyPrograms.size()
> printf "Current ESP: 0x%x\n", $esp
> continue
> end

# 现在每次中断都会自动打印信息并继续
(gdb) continue

# 你会看到输出像：
# Interrupt! Current thread: first_thread (PID 0)
# Ready queue size: 3
# Current ESP: 0x1000000
#
# Interrupt! Current thread: second_thread (PID 1)
# Ready queue size: 3
# Current ESP: 0x2000000
#
# ...
```

## 演示 4: 观察线程执行完毕和清理

### 步骤

```bash
# 在线程退出处设置断点
(gdb) break program_exit
(gdb) continue

# 当一个线程执行完毕时会停止
(gdb) p programManager.running->name
$18 = "third_thread"

# 设置线程状态为 DEAD
(gdb) step

(gdb) p programManager.running->status
$19 = DEAD

# 继续，观察下一个线程被选中
(gdb) step
(gdb) p programManager.running->name
$20 = "first_thread"  # 新线程接管
```

## 关键观察点

### 1. 栈指针 (ESP) 的变化

每个线程有自己的 4KB 栈空间。线程切换时，ESP 从一个栈切换到另一个栈。

```cpp
// 在 PCB 中
int *stack;  // 指向栈顶部
```

### 2. 指令指针 (EIP) 的恢复

线程中断时，EIP 被保存到栈中。线程恢复时，`iret` 指令会从栈中恢复 EIP。

### 3. 寄存器保存/恢复

虽然现在的实现相对简单，但完整的线程切换应该保存所有通用寄存器。

## 高级调试技巧

### 1. 内存断点 (Watch)

```bash
# 监视某个变量的变化
(gdb) watch programManager.running->ticks
(gdb) continue
# 当变量改变时停止
```

### 2. 条件断点

```bash
# 仅当特定条件满足时触发
(gdb) break schedule if programManager.running->pid == 1
(gdb) continue
# 仅当 PID 为 1 的线程调度时停止
```

### 3. 自定义命令

```bash
(gdb) define show_all_threads
> printf "=== All Threads ===\n"
> printf "Running: %s (PID %d, Status %d)\n", programManager.running->name, programManager.running->pid, programManager.running->status
> printf "Ready queue size: %d\n", programManager.readyPrograms.size()
> end

(gdb) show_all_threads
```

## 预期结果总结

通过上述演示，你应该能够观察到：

1. ✓ 新线程的栈被正确初始化
2. ✓ 线程通过时间中断被周期性地中断
3. ✓ 调度器选择下一个线程
4. ✓ ESP 和其他寄存器被正确切换
5. ✓ 新线程从被中断处恢复执行
6. ✓ 线程执行完毕后被标记为 DEAD 并清理

这些观察验证了操作系统并发执行的核心原理：**通过硬件中断和上下文切换来实现在单个处理器上执行多个线程**。

## 参考资源

- GDB 官方文档：https://sourceware.org/gdb/documentation/
- x86 架构手册中的中断和异常
- Linux 内核线程实现细节

---

**完成 Assignment 3 需要：**
- [x] 理解时间中断驱动的调度机制
- [x] 使用 GDB 跟踪线程创建过程
- [x] 使用 GDB 观察线程切换过程
- [x] 理解栈和寄存器的角色
- [x] 验证调度决策和线程状态变化
