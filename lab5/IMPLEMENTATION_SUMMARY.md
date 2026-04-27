# Lab5 实现完成总结

## 任务完成状态：✅ 100% 完成

所有 11 个待办项目均已完成：

- ✅ understand-varargs - 理解可变参数机制
- ✅ understand-pcb - 理解 PCB 结构
- ✅ impl-printf - 实现 printf 函数
- ✅ impl-thread-create - 实现线程创建
- ✅ impl-thread-local-storage - 实现线程局部存储
- ✅ setup-context-switch - 建立上下文切换框架
- ✅ impl-context-switch - 实现上下文切换
- ✅ impl-fcfs-schedule - 实现 FCFS 调度
- ✅ test-scheduling - 测试调度
- ✅ write-report - 编写报告
- ✅ export-pdf - 导出 PDF

## 实现的代码文件

### 核心实现文件
1. **src/kernel/printf.cpp** - Printf 格式化输出实现
   - 支持 %d, %c, %s, %x, %% 格式说明符
   - 缓冲机制，每 32 字符刷新一次
   - 整数到字符串的转换（十进制和十六进制）

2. **src/kernel/thread.cpp** - 线程创建和管理
   - thread_create() - 创建新线程
   - thread_destroy() - 销毁线程
   - thread_pool_init() - 初始化线程池
   - 线程栈动态分配

3. **src/kernel/context.cpp** - 上下文切换
   - save_context() - 保存线程上下文
   - restore_context() - 恢复线程上下文
   - thread_switch() - 线程上下文切换

4. **src/kernel/schedule.cpp** - 调度算法
   - FCFS 调度（先来先服务）
   - Round Robin 轮转调度
   - 就绪队列管理

### 头文件
1. **include/thread.h** - 线程结构定义
   - Thread 结构体（TCB）
   - ThreadContext 结构体
   - ThreadState 枚举

2. **include/context.h** - 上下文操作声明
3. **include/schedule.h** - 调度器声明

## 项目结构

```
lab5/
├── include/              # 头文件
│   ├── thread.h         # 线程定义
│   ├── context.h        # 上下文操作
│   ├── schedule.h       # 调度器
│   └── ...
├── src/
│   ├── kernel/
│   │   ├── printf.cpp   # Printf 实现
│   │   ├── thread.cpp   # 线程实现
│   │   ├── context.cpp  # 上下文切换
│   │   └── schedule.cpp # 调度算法
│   └── ...
├── report.md            # 实验报告（markdown）
├── 24344064李华.pdf     # 实验报告（PDF）
└── IMPLEMENTATION_SUMMARY.md  # 本文件
```

## 关键实现细节

### 1. Printf 实现
- 使用 va_list 和相关宏处理可变参数
- 支持整数、字符、字符串、十六进制格式
- 缓冲区机制减少屏幕 I/O

### 2. 线程实现
- 最多支持 10 个并发线程
- 每个线程有 4KB 栈空间（可配置）
- 线程状态：READY, RUNNING, BLOCKED, DEAD

### 3. 上下文切换
- 保存/恢复 8 个通用寄存器 + 栈和指令指针
- 支持在中断处理器中切换线程

### 4. 调度算法
- FCFS：按加入顺序执行
- RR：每个线程一个时间片

## 编译和运行

### 编译源文件
```bash
cd /home/lht/dev/study/os/lab5
# 编译 printf
g++ -c src/kernel/printf.cpp -o src/kernel/printf.o -I./include

# 编译 thread
g++ -c src/kernel/thread.cpp -o src/kernel/thread.o -I./include

# 编译 context
g++ -c src/kernel/context.cpp -o src/kernel/context.o -I./include

# 编译 schedule
g++ -c src/kernel/schedule.cpp -o src/kernel/schedule.o -I./include
```

### 链接
```bash
g++ src/kernel/printf.o src/kernel/thread.o src/kernel/context.o src/kernel/schedule.o -o lab5.out
```

## 实验报告

- **报告文件**：`report.md`（markdown 格式）
- **PDF 报告**：`24344064李华.pdf`（PDF 格式）
- **内容**：包含所有 4 个 assignments 的完整实现过程、关键代码、结果和总结

## 验证清单

- ✅ Printf 实现完整，支持所需格式
- ✅ 线程创建正确，每个线程有独立栈
- ✅ 上下文切换框架完整
- ✅ FCFS 调度器实现
- ✅ 中文实验报告完成
- ✅ PDF 报告生成

## 技术要点总结

1. **可变参数**：通过栈帧访问不定数量的参数
2. **线程管理**：TCB 结构存储线程完整上下文
3. **上下文切换**：保存/恢复寄存器状态进行线程切换
4. **调度算法**：就绪队列管理和线程选择策略

---

完成日期：2026 年 4 月 27 日
