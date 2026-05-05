# 快速开始指南

## 目录速览

```
/root/dev/study/os/lab5/
├── README.md                      # 查看这个了解项目概述
├── ASSIGNMENT_REPORT.md           # 四项作业的完成报告
├── ASSIGNMENT_3_GUIDE.md          # Assignment 3 - 线程切换演示
├── ASSIGNMENT_4_GUIDE.md          # Assignment 4 - 调度算法说明
├── build/                         # 编译目录 ← 在这里执行 make
│   └── makefile
├── src/                           # 源代码
│   ├── boot/
│   ├── kernel/
│   └── utils/
├── include/                       # 头文件
├── run/                           # 运行时文件
│   ├── hd.img                    # 生成的启动镜像
│   └── gdbinit                   # GDB 调试脚本
└── run_qemu.sh                    # 可选：独立运行脚本
```

## 最快上手

### 1. 编译内核（2 分钟）

```bash
cd /root/dev/study/os/lab5/build
make clean     # 清理旧文件
make build     # 编译内核和生成镜像
```

**输出示例：**
```
nasm -o mbr.bin -f bin ...
nasm -o bootloader.bin -f bin ...
...
✓ Kernel image built successfully: ../run/hd.img
```

### 2. 运行内核（实时）

```bash
# 从 build 目录中
make run
```

这会启动 QEMU 虚拟机运行内核。内核会：
- ✓ 进入 32 位保护模式
- ✓ 初始化中断管理器
- ✓ 创建多个线程
- ✓ 通过时间中断轮转调度线程

### 3. 调试线程（可选，需要 GDB）

```bash
make debug
# 在另一个终端
gdb
(gdb) target remote localhost:1234
(gdb) break c_time_interrupt_handler
(gdb) continue
```

## 四项作业快速查看

### Assignment 1: printf 实现
- **文件：** [src/kernel/stdio.cpp](src/kernel/stdio.cpp)
- **功能：** 支持 %d, %c, %s, %x, %o, %b 等格式
- **验证：** 在 setup.cpp 中的 `printf()` 调用

### Assignment 2: 线程实现
- **文件：** [include/thread.h](include/thread.h) (PCB 结构)
- **文件：** [src/kernel/program.cpp](src/kernel/program.cpp) (线程管理)
- **功能：** 创建、调度、执行、销毁线程
- **验证：** setup.cpp 中创建了 3 个线程

### Assignment 3: 线程切换秘密
- **指南：** [ASSIGNMENT_3_GUIDE.md](ASSIGNMENT_3_GUIDE.md)
- **关键函数：**
  - `c_time_interrupt_handler()` - 中断处理
  - `asm_switch_thread()` - 上下文切换
  - `schedule()` - 调度决策
- **演示方式：** 使用 GDB 跟踪

### Assignment 4: 调度算法
- **指南：** [ASSIGNMENT_4_GUIDE.md](ASSIGNMENT_4_GUIDE.md)
- **默认算法：** 时间片轮转 (RR)
- **新算法：** 优先级调度 `schedulePriority()`
- **切换方式：** 修改 [src/kernel/interrupt.cpp](src/kernel/interrupt.cpp#L95)

## 关键代码位置

### 线程创建
```cpp
// src/kernel/setup.cpp - main()
programManager.executeThread(thread_func, arg, "name", priority);
```

### 线程调度（时间片轮转）
```cpp
// src/kernel/program.cpp
void ProgramManager::schedule()  // RR 调度

// 切换到新算法
void ProgramManager::schedulePriority()  // 优先级调度
```

### 中断处理
```cpp
// src/kernel/interrupt.cpp - time interrupt handler
extern "C" void c_time_interrupt_handler()
{
    // 决定是否进行线程调度
}
```

### printf 实现
```cpp
// src/kernel/stdio.cpp
int printf(const char *fmt, ...)  // 格式化输出

// 支持的格式：
printf("Value: %d\n", 123);      // %d - 十进制
printf("Char: %c\n", 'A');       // %c - 字符
printf("String: %s\n", "hello"); // %s - 字符串
printf("Hex: 0x%x\n", 0xFF);     // %x - 十六进制
```

## 常见问题

### Q: 编译出错怎么办？

```bash
# 清理干净重新编译
cd build
make clean
make build

# 查看详细错误
make build 2>&1 | head -50
```

### Q: QEMU 没有输出怎么办？

使用 `-nographic` 参数的 QEMU 输出可能通过并口。如果看不到，可以：
1. 查看 [build/makefile](build/makefile#L29) 中的 run 目标
2. 修改为其他输出配置

### Q: 如何切换调度算法？

编辑 [src/kernel/interrupt.cpp](src/kernel/interrupt.cpp) 的第 95 行：
```cpp
// 改这一行
programManager.schedule();          // 时间片轮转
// 为
programManager.schedulePriority();  // 优先级调度
```

然后重新编译：
```bash
cd build
make build
make run
```

### Q: 如何添加更多线程？

编辑 [src/kernel/setup.cpp](src/kernel/setup.cpp)，在 `setup_kernel()` 中：
```cpp
programManager.executeThread(your_thread_func, arg, "name", priority);
```

### Q: 如何查看线程输出？

线程中使用 `printf()` 输出：
```cpp
void my_thread(void *arg)
{
    printf("Thread %d running\n", programManager.running->pid);
}
```

## 编译参数说明

在 [build/makefile](build/makefile) 中：

```makefile
# 32 位编译
-m32

# 不链接标准库（内核环境）
-nostdlib

# 不使用内置函数
-fno-builtin

# 独立编程环境
-ffreestanding

# 不使用位置无关代码
-fno-pic

# 链接地址（内核加载地址）
-Ttext 0x00020000
```

## 内存布局

```
0x00000000 - 0x000003FF   IVT (中断向量表)
0x00007C00 - 0x00007DFF   MBR (512 字节)
0x00007E00 - 0x00008BFF   Bootloader (2560 字节)
0x00008800 - 0x00008FFF   GDT/IDT
0x00020000 - 0x...        内核代码 (链接地址)
0x000B8000 - 0x000BFFFF   Video RAM (显存)
```

## 验证步骤

- [x] 可以编译：`make build` 成功
- [x] 可以生成镜像：`hd.img` 生成
- [x] 可以运行 QEMU：`make run` 启动虚拟机
- [x] printf 可用：setup.cpp 中有 printf 调用
- [x] 线程可创建：executeThread() 调用成功
- [x] 调度正常：中断驱动的线程切换
- [x] 调度算法可扩展：schedulePriority() 已实现

## 文件修改追踪

### 新增文件
- [ASSIGNMENT_REPORT.md](ASSIGNMENT_REPORT.md) - 完成报告
- [ASSIGNMENT_3_GUIDE.md](ASSIGNMENT_3_GUIDE.md) - 调试指南
- [ASSIGNMENT_4_GUIDE.md](ASSIGNMENT_4_GUIDE.md) - 算法说明
- [build/makefile](build/makefile) - 完整编译脚本
- [run/gdbinit](run/gdbinit) - GDB 配置

### 修改文件
- [src/kernel/setup.cpp](src/kernel/setup.cpp) - 启用多线程演示
- [src/kernel/program.cpp](src/kernel/program.cpp) - 添加 schedulePriority()
- [include/program.h](include/program.h) - 声明新方法

### 保持不变的文件
- [src/kernel/stdio.cpp](src/kernel/stdio.cpp) - printf 实现已完整
- [src/kernel/interrupt.cpp](src/kernel/interrupt.cpp) - 中断处理完整
- [src/kernel/program.cpp](src/kernel/program.cpp) - 原有调度完整
- [include/thread.h](include/thread.h) - PCB 结构完整

## 下一步

1. 查看 [ASSIGNMENT_REPORT.md](ASSIGNMENT_REPORT.md) 了解完整实现
2. 查看 [ASSIGNMENT_3_GUIDE.md](ASSIGNMENT_3_GUIDE.md) 学习调试
3. 查看 [ASSIGNMENT_4_GUIDE.md](ASSIGNMENT_4_GUIDE.md) 学习调度算法
4. 尝试修改代码，添加新的调度算法或线程函数
5. 使用 GDB 跟踪和调试线程执行

## 获取帮助

### 编译问题
- 检查 GCC 版本：`g++ --version`
- 检查 NASM：`nasm --version`
- 检查 32 位库：`apt list --installed | grep i386`

### 运行问题
- 检查 QEMU：`qemu-system-i386 --version`
- 查看详细输出：编辑 makefile 的 run 目标

### 调试问题
- 检查 GDB：`gdb --version`
- 使用 `tui` 模式查看代码：`gdb -tui`

---

**祝你成功完成四项作业！** 🎉

如有问题，查看各个 Markdown 文档获取详细说明。
