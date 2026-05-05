# Lab5 - 内核线程实验完成报告

## 项目概述
本项目在参考材料 src/4 的基础上，完成了四项作业的实现，包括 printf 格式化输出、内核线程管理、线程调度切换以及调度算法改进。

## 构建和运行

### 快速开始
```bash
cd /root/dev/study/os/lab5/build
make build    # 编译内核
make run      # 运行 QEMU 虚拟机
```

### 文件结构
```
lab5/
├── build/              # 编译目录 (执行 make 的地方)
│   └── makefile       # 完整的内核编译脚本
├── src/
│   ├── boot/          # 引导程序和内核入口
│   │   ├── mbr.asm
│   │   ├── bootloader.asm
│   │   └── entry.asm
│   ├── kernel/        # 内核源代码
│   │   ├── setup.cpp        # 内核初始化和线程创建
│   │   ├── interrupt.cpp    # 中断管理和时间中断处理
│   │   ├── program.cpp      # 线程管理和调度算法
│   │   └── stdio.cpp        # printf 实现
│   └── utils/         # 工具函数
│       ├── asm_utils.asm
│       ├── list.cpp         # 链表数据结构
│       └── stdlib.cpp
├── include/           # 头文件
├── run/               # 运行时文件
│   ├── hd.img        # 生成的启动镜像
│   └── gdbinit       # GDB 调试脚本
└── README.md         # 本文件
```

## 四项作业完成情况

### Assignment 1: Printf 的实现 ✓

**完成内容：**
- 在 [src/kernel/stdio.cpp](src/kernel/stdio.cpp) 中实现了格式化输出函数 `printf`
- 支持的格式说明符：
  - `%d` - 十进制整数
  - `%c` - 单个字符
  - `%s` - 字符串
  - `%x` - 十六进制整数
  - `%o` - 八进制整数
  - `%b` - 二进制整数
  - `%%` - 字面量百分号

**核心实现：**
- 使用 `<stdarg.h>` 中的可变参数机制 (`va_list`, `va_start`, `va_arg`)
- 通过缓冲区优化输出性能
- 将每个参数解析并格式化后输出到屏幕

**使用示例：**
```cpp
printf("Thread %d: iteration %d\n", pid, i);
printf("Value in hex: 0x%x, in decimal: %d\n", 0x1234, 4660);
```

---

### Assignment 2: 线程的实现 ✓

**完成内容：**
- 设计完整的 PCB (Process Control Block) 结构，包含：
  - `stack` - 栈指针
  - `name` - 线程名
  - `status` - 线程状态 (CREATED, RUNNING, READY, BLOCKED, DEAD)
  - `priority` - 优先级
  - `pid` - 进程/线程 ID
  - `ticks` - 时间片
  - `ticksPassedBy` - 已执行时间

**核心实现 ([src/kernel/program.cpp](src/kernel/program.cpp))：**
- `executeThread()` - 创建新线程，分配 PCB 和栈空间，初始化执行上下文
- `schedule()` - 时间片轮转调度算法
- `program_exit()` - 线程退出处理

**线程创建流程：**
1. 分配 4KB PCB 空间
2. 初始化线程栈，设置返回地址和参数
3. 将线程加入就绪队列
4. 中断驱动的调度器会在适当时刻切换到新线程

**当前测试程序 ([src/kernel/setup.cpp](src/kernel/setup.cpp))：**
- 创建 3 个线程（first_thread, second_thread, third_thread）
- 每个线程执行 3 次迭代并打印消息
- 演示时间片轮转调度

---

### Assignment 3: 线程调度切换的秘密 ✓

**完成内容：**
- 通过中断驱动的调度实现线程上下文切换
- 使用 GDB 调试和跟踪线程切换过程

**关键函数：**
- `c_time_interrupt_handler()` [interrupt.cpp](src/kernel/interrupt.cpp#L88) - 时间中断处理程序
  - 递减当前线程的时间片计数
  - 时间片用尽时调用 `schedule()` 进行线程切换

- `asm_switch_thread(PCB *cur, PCB *next)` [src/utils/asm_utils.asm](src/utils/asm_utils.asm) - 汇编实现的线程切换
  - 保存当前线程的 ESP 到 PCB
  - 加载下一个线程的 ESP
  - 实现硬件级的上下文切换

**线程切换过程：**
1. **中断发生** - 时钟中断打断当前线程执行
2. **保存状态** - CPU 自动压栈 (CS, EIP, EFLAGS 等)
3. **进入中断处理** - 执行 `c_time_interrupt_handler()`
4. **调度决策** - 如果时间片用尽，调用 `schedule()`
5. **上下文切换** - `asm_switch_thread()` 交换栈指针和所有寄存器
6. **线程恢复** - 加载新线程的栈，`iret` 返回并继续执行

**GDB 调试步骤：**

启动调试模式：
```bash
cd build
make debug
# 在另一个终端执行
gdb -q
(gdb) file kernel.o
(gdb) target remote :1234
(gdb) break c_time_interrupt_handler
(gdb) break asm_switch_thread
(gdb) continue
(gdb) info registers    # 查看寄存器变化
(gdb) backtrace         # 查看调用栈
(gdb) info threads      # 查看线程信息
```

---

### Assignment 4: 调度算法的实现 ✓

**完成内容：**
- 保留原有的 **时间片轮转 (RR) 调度** 算法
- 新增 **优先级调度** 算法 `schedulePriority()`

**时间片轮转 (RR) 调度 - `schedule()`：**
- 算法：队列首部取线程，执行完时间片后放回队尾
- 特点：公平性好，响应速度均衡
- 实现：[src/kernel/program.cpp](src/kernel/program.cpp#L74-L110)
```cpp
// 当前线程时间片用尽时
running->status = ProgramStatus::READY;
running->ticks = running->priority * 10;  // 重置时间片
readyPrograms.push_back(&(running->tagInGeneralList));  // 加入队尾

// 选择就绪队列的第一个线程
ListItem *item = readyPrograms.front();
PCB *next = ListItem2PCB(item, tagInGeneralList);
```

**优先级调度 - `schedulePriority()` [新实现]：**
- 算法：始终选择优先级最高的就绪线程执行
- 特点：高优先级任务优先执行，可能导致低优先级任务饥荒
- 实现：[src/kernel/program.cpp](src/kernel/program.cpp#L114-L160)
```cpp
// 遍历就绪队列，找到优先级最高的线程
while (current != &(readyPrograms.head))
{
    PCB *thread = ListItem2PCB(current, tagInGeneralList);
    if (thread->priority > highest->priority)
    {
        highest = thread;
        item = current;
    }
    current = current->next;
}
// 选择优先级最高的线程
readyPrograms.erase(item);
```

**使用方式：**
- 在 [src/kernel/interrupt.cpp](src/kernel/interrupt.cpp#L88) 的中断处理中：
  ```cpp
  // 切换算法只需改一行：
  // programManager.schedule();           // 时间片轮转
  programManager.schedulePriority();      // 优先级调度
  ```

**对比：**

| 特性 | 时间片轮转 | 优先级调度 |
|------|----------|----------|
| 公平性 | 高 | 低 |
| 响应时间 | 均衡 | 高优先级快 |
| 实现复杂度 | 低 | 中等 |
| 适用场景 | 通用 | 实时系统 |

---

## 编译配置

### Makefile 细节

**编译步骤：**
1. **16位引导程序** - 生成原始二进制
   ```makefile
   nasm -o mbr.bin -f bin -I../include/ ../src/boot/mbr.asm
   ```

2. **32位内核代码** - 生成 ELF32 对象文件
   ```makefile
   g++ -m32 -nostdlib -ffreestanding ... -c ../src/kernel/setup.cpp -o setup.o
   nasm -f elf32 ../src/utils/asm_utils.asm -o asm_utils.o
   ```

3. **链接** - 生成可启动镜像
   ```makefile
   ld -o kernel.bin -melf_i386 -N entry.obj setup.o ... -e enter_kernel -Ttext 0x00020000 --oformat binary
   ```

4. **镜像生成** - 使用 dd 组合各部分
   ```makefile
   dd if=mbr.bin of=../run/hd.img bs=512 count=1 seek=0 conv=notrunc
   dd if=bootloader.bin of=../run/hd.img bs=512 count=5 seek=1 conv=notrunc
   dd if=kernel.bin of=../run/hd.img bs=512 count=145 seek=6 conv=notrunc
   ```

### 内存布局

```
0x00000000 - 0x000003FF   IVT (中断向量表)
0x00007C00 - 0x00007DFF   MBR (第一级引导)
0x00007E00 - 0x00008BFF   Bootloader (第二级引导)
0x00008800 - 0x00008FFF   GDT/IDT
0x00020000 - ...          内核代码和数据 (链接地址)
0x000B8000 - 0x000BFFFF   Video RAM (显存)
```

---

## 运行结果

编译后运行内核：
```bash
cd /root/dev/study/os/lab5/build
make run
```

**预期输出：**
- 内核引导程序加载内核到内存
- 进入 32 位保护模式
- 初始化中断管理器和时间中断
- 创建三个线程并启动调度
- 线程通过时间中断轮转执行
- 所有线程执行完毕后系统停止

---

## 主要改进和特点

1. **完整的内核镜像编译** - 从源代码到可启动的 QEMU 镜像的完整工作流
2. **三层线程体系** - PCB 数据结构 + 内核调度器 + 硬件中断驱动
3. **多算法支持** - 易于扩展新的调度算法
4. **可调试性** - 支持 GDB 调试，可观察线程状态和上下文切换

---

## 参考资料

- OS 实验教材：《操作系统真象还原》
- NASM 汇编语言文档
- GCC 交叉编译工具链
- QEMU 虚拟机文档

---

## 验证清单

- [x] Assignment 1: printf 实现完成，支持多种格式说明符
- [x] Assignment 2: 线程创建和管理完成，支持优先级设置
- [x] Assignment 3: 线程切换秘密通过时间中断和 GDB 调试理解
- [x] Assignment 4: 时间片轮转和优先级调度两种算法都已实现
- [x] 项目可编译：`make build` 成功
- [x] 项目可运行：`make run` 启动 QEMU
- [x] 所有代码基于参考材料修改，未过度自造轮子

---

**完成日期：** 2026-05-04  
**项目路径：** /root/dev/study/os/lab5/
