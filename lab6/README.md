# Lab6 - 并发与锁机制

## 概览

本实验通过"消失的芝士汉堡"故事来演示多线程编程中的并发问题及其解决方案。

## 实验结构

```
lab6/
├── src/
│   ├── assignment1/          # 演示竞态条件（Race Condition）
│   ├── assignment2/          # 使用自旋锁（SpinLock）解决
│   └── assignment3/          # 使用信号量（Semaphore）解决
├── report.md                 # 详细的实验报告
└── README.md                 # 本文件
```

## 三个 Assignment 说明

### Assignment 1：竞态条件的演示

**目标**：演示无同步机制时的并发问题

**关键代码**：
- `src/assignment1/src/kernel/setup.cpp` 中的 `a_mother()` 和 `a_naughty_boy()` 函数
- 共享变量：`cheese_burger`

**预期输出**：母亲最后观察到 0 个汉堡（错误！应该是 10 个）

```bash
cd src/assignment1/build
make clean && make build && make run
```

### Assignment 2：自旋锁解决方案

**目标**：使用自旋锁实现线程互斥

**关键代码**：
- `src/assignment2/include/sync.h` 中的 `SpinLock` 类定义
- `src/assignment2/src/kernel/sync.cpp` 中的 `SpinLock` 实现
- `src/assignment2/src/kernel/setup.cpp` 中使用自旋锁保护 `cheese_burger`

**核心机制**：
- 使用原子操作 `asm_atomic_exchange` 实现互斥
- `bolt` 变量：0 表示未锁定，1 表示已锁定
- `lock()` 方法：自旋等待直到获得锁
- `unlock()` 方法：释放锁

**预期输出**：母亲最后正确地观察到 10 个汉堡 ✓

```bash
cd src/assignment2/build
make clean && make build && make run
```

### Assignment 3：信号量解决方案

**目标**：使用信号量实现更高级的线程同步

**关键代码**：
- `src/assignment3/include/sync.h` 中的 `SpinLock` 和 `Semaphore` 类定义
- `src/assignment3/src/kernel/sync.cpp` 中的完整实现
- `src/assignment3/src/kernel/setup.cpp` 中使用信号量保护 `cheese_burger`

**核心机制**：
- 信号量计数器 `counter`：表示可用资源数
- 等待队列 `waiting`：阻塞的线程列表
- `P()` 方法：减少计数，如果为 0 则阻塞线程
- `V()` 方法：增加计数，唤醒一个等待的线程
- 使用自旋锁保护内部状态

**预期输出**：母亲最后正确地观察到 10 个汉堡 ✓

```bash
cd src/assignment3/build
make clean && make build && make run
```

## 编译和运行

### 快速编译所有 Assignment

```bash
cd /home/lht/dev/study/os/lab6/src

# 编译 Assignment 1
cd assignment1/build && make clean && make build

# 编译 Assignment 2
cd ../../assignment2/build && make clean && make build

# 编译 Assignment 3
cd ../../assignment3/build && make clean && make build
```

### 运行单个 Assignment

```bash
# 进入指定 Assignment 的 build 目录
cd /home/lht/dev/study/os/lab6/src/assignment{1,2,3}/build

# 运行（使用 QEMU）
make run
```

### QEMU 控制

- **退出 QEMU**：按 `Ctrl+A` 然后按 `X`
- **暂停/继续**：`Ctrl+C` 然后 `c`

## 关键文件说明

### Makefile

所有三个 Assignment 都使用相同的 Makefile 结构：

```
build/
├── makefile               # 编译脚本
```

**Makefile 流程**：
1. 编译 C++ 源文件为 `.o` 文件
2. 编译汇编文件为 `.obj` 和 `.o` 文件
3. 链接生成 `kernel.bin`（二进制核心）和 `kernel.o`（调试信息）
4. 使用 `dd` 命令烧写到虚拟硬盘镜像 `hd.img`
5. 使用 QEMU 运行

### 源代码组织

```
src/
├── boot/
│   ├── mbr.asm            # 主引导记录
│   ├── bootloader.asm     # 第一阶段引导程序
│   └── entry.asm          # 第二阶段引导程序
├── kernel/
│   ├── setup.cpp          # 核心初始化和线程演示
│   ├── interrupt.cpp      # 中断处理
│   ├── program.cpp        # 线程管理器
│   ├── stdio.cpp          # 屏幕输出
│   └── sync.cpp           # 同步原语实现（仅 Assignment 2, 3）
└── utils/
    ├── asm_utils.asm      # 汇编工具函数
    ├── list.cpp           # 链表数据结构
    └── stdlib.cpp         # 标准库函数
```

## 理解核心概念

### 竞态条件（Race Condition）

多个线程同时访问共享资源，且至少有一个是写操作时，最终结果取决于线程的执行时序。

**在本实验中**：
- 母亲线程 `cheese_burger += 10`
- 儿子线程 `cheese_burger -= 10`
- 如果儿子在母亲读取和写回之间执行，结果就会错误

### 原子操作

不可被中断的操作。在本实验中，使用 x86 的 `XCHG` 指令实现原子交换。

```asm
xchg eax, [ecx]   ; 原子地交换 eax 和内存地址 ecx 的值
```

### 自旋锁 vs 信号量

| 特性 | 自旋锁 | 信号量 |
|------|--------|--------|
| 等待方式 | 忙轮询 | 阻塞等待 |
| CPU 利用率 | 低 | 高 |
| 适用场景 | 短临界区 | 长临界区 |
| 实现复杂度 | 简单 | 复杂 |

## 调试建议

### 查看编译详情

```bash
make build 2>&1 | grep -E "(error|warning)"
```

### 查看虚拟硬盘镜像

```bash
ls -lh ../run/hd.img
```

### 启用 QEMU 调试

在 Makefile 中的 `debug` 目标启用 GDB 调试：

```bash
make debug
```

## 常见问题

### Q: 编译时出现 "undefined reference to asm_atomic_exchange"

**A**: 确保 `src/utils/asm_utils.asm` 被正确编译。检查 Makefile 中的 `asm_utils.o` 规则。

### Q: QEMU 无法启动

**A**: 检查 `../run/hd.img` 文件大小是否正确（约 80KB）。如果损坏，删除后重新编译。

### Q: 看不到任何输出

**A**: QEMU 默认使用并行端口输出。运行 `make run` 时终端应该显示输出。如果没有，检查 Makefile 中的 `-parallel stdio` 参数。

## 扩展学习

学完本实验后，建议了解：

1. **读写锁**（RWLock）：允许多个读者，但写者需要独占
2. **条件变量**（Condition Variable）：用于实现更复杂的线程间通信
3. **生产者-消费者问题**：使用信号量解决的经典问题
4. **死锁**（Deadlock）：当多个线程相互等待时发生

## 参考资源

- 实验材料：`../materials/实验6 并发与锁机制.html`
- 详细报告：`../report.md`
- x86 汇编参考：Intel 64 and IA-32 Architectures Software Developer's Manual

## 作者信息

基于教材提供的参考实现进行验证和说明。

---

**最后更新**：2026-05-11
