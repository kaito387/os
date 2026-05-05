# 📚 Lab5 项目文档导航

## 🎯 不同用户的快速导航

### 👶 初学者 - 从这里开始

1. **首先阅读：** [QUICKSTART.md](QUICKSTART.md)
   - 快速了解项目
   - 编译和运行步骤
   - 常见问题解答

2. **然后阅读：** [COMPLETION_SUMMARY.md](COMPLETION_SUMMARY.md)
   - 项目整体概览
   - 四项作业的完成状态
   - 核心实现要点

3. **最后查看：** [ASSIGNMENT_REPORT.md](ASSIGNMENT_REPORT.md)
   - 每项作业的详细说明
   - 代码实现原理
   - 核心函数位置

### 👨‍💻 有经验的开发者 - 直达主题

- **想快速编译？** → [QUICKSTART.md#最快上手](QUICKSTART.md#最快上手)
- **想学 printf？** → [ASSIGNMENT_REPORT.md#assignment-1-printf-的实现](ASSIGNMENT_REPORT.md#assignment-1-printf-的实现)
- **想理解线程？** → [ASSIGNMENT_REPORT.md#assignment-2-线程的实现](ASSIGNMENT_REPORT.md#assignment-2-线程的实现)
- **想调试线程？** → [ASSIGNMENT_3_GUIDE.md](ASSIGNMENT_3_GUIDE.md)
- **想学调度算法？** → [ASSIGNMENT_4_GUIDE.md](ASSIGNMENT_4_GUIDE.md)

### 🎓 学生做作业 - 按难度顺序

1. **简单（1 小时）**
   - 编译项目
   - 运行 QEMU
   - 阅读代码
   - 参考：[QUICKSTART.md](QUICKSTART.md)

2. **中等（2-3 小时）**
   - 理解 printf 实现
   - 理解线程创建流程
   - 理解调度机制
   - 参考：[ASSIGNMENT_REPORT.md](ASSIGNMENT_REPORT.md)

3. **困难（4-5 小时）**
   - 使用 GDB 调试线程切换
   - 理解中断处理流程
   - 实现新的调度算法
   - 参考：[ASSIGNMENT_3_GUIDE.md](ASSIGNMENT_3_GUIDE.md) 和 [ASSIGNMENT_4_GUIDE.md](ASSIGNMENT_4_GUIDE.md)

---

## 📖 文档详细说明

### [QUICKSTART.md](QUICKSTART.md) - 快速开始指南
**适合人群：** 所有人  
**阅读时间：** 5-10 分钟  
**包含内容：**
- 目录速览
- 快速编译和运行步骤
- 四项作业快速查看
- 常见问题解答
- 编译参数说明

**何时查看：** 第一次使用项目

---

### [COMPLETION_SUMMARY.md](COMPLETION_SUMMARY.md) - 项目完成总结
**适合人群：** 想要整体了解的人  
**阅读时间：** 15-20 分钟  
**包含内容：**
- 项目完成状态表
- 快速开始（编译/运行/调试）
- 项目结构详图
- 四项作业核心实现要点
- 代码规模统计
- 关键函数清单
- 验证清单
- 可能的扩展方向

**何时查看：** 想要快速了解整个项目

---

### [ASSIGNMENT_REPORT.md](ASSIGNMENT_REPORT.md) - 完整的作业报告
**适合人群：** 学生、教师、技术人员  
**阅读时间：** 30-45 分钟  
**包含内容：**
- 四项作业的完整说明
- 每项作业的实现原理
- 关键代码片段
- 文件路径和行号
- 编译配置详解
- 内存布局图
- 运行结果说明
- 参考资料

**何时查看：** 需要详细了解作业实现

---

### [ASSIGNMENT_3_GUIDE.md](ASSIGNMENT_3_GUIDE.md) - 线程切换演示指南
**适合人群：** 想要深入理解线程的人  
**阅读时间：** 20-30 分钟  
**包含内容：**
- 线程切换原理（5 个步骤）
- GDB 基本命令回顾
- 4 个具体演示方案
  - 演示 1：观察新线程的创建和启动
  - 演示 2：观察时间中断处理和线程切换
  - 演示 3：连续观察多次切换
  - 演示 4：观察线程执行完毕和清理
- 关键观察点
- 高级调试技巧
- 预期结果总结

**何时查看：** 做 Assignment 3 或想要深入理解线程

---

### [ASSIGNMENT_4_GUIDE.md](ASSIGNMENT_4_GUIDE.md) - 调度算法详细说明
**适合人群：** 想要学习调度算法的人  
**阅读时间：** 25-35 分钟  
**包含内容：**
- 已实现的 2 种算法（RR 和优先级）
- 每种算法的完整代码
- 算法特点对比表
- 应用场景分析
- 切换算法的方法
- 测试不同算法的方法
- 其他可能算法的简介
- 性能比较表
- 添加新算法的步骤
- 调试和验证方法

**何时查看：** 做 Assignment 4 或想要学习调度算法

---

## 🔍 按主题查找文档

### 📍 编译相关
- 快速编译步骤 → [QUICKSTART.md#最快上手](QUICKSTART.md#最快上手)
- 详细编译配置 → [COMPLETION_SUMMARY.md#-编译配置](COMPLETION_SUMMARY.md#-编译配置)
- 编译流程说明 → [ASSIGNMENT_REPORT.md#编译配置](ASSIGNMENT_REPORT.md#编译配置)

### 🚀 运行相关
- 快速运行步骤 → [QUICKSTART.md#最快上手](QUICKSTART.md#最快上手)
- 运行和调试 → [COMPLETION_SUMMARY.md#快速开始](COMPLETION_SUMMARY.md#快速开始)
- 常见问题 → [QUICKSTART.md#常见问题](QUICKSTART.md#常见问题)

### 🧵 线程相关
- 线程实现详解 → [ASSIGNMENT_REPORT.md#assignment-2-线程的实现](ASSIGNMENT_REPORT.md#assignment-2-线程的实现)
- 线程切换原理 → [ASSIGNMENT_3_GUIDE.md#目标](ASSIGNMENT_3_GUIDE.md#目标)
- 线程调试方法 → [ASSIGNMENT_3_GUIDE.md#演示-1-观察新线程的创建和启动](ASSIGNMENT_3_GUIDE.md#演示-1-观察新线程的创建和启动)
- PCB 结构详解 → [COMPLETION_SUMMARY.md#2️⃣-assignment-2-线程实现](COMPLETION_SUMMARY.md#2️⃣-assignment-2-线程实现)

### 📤 Printf 相关
- Printf 使用示例 → [COMPLETION_SUMMARY.md#1️⃣-assignment-1---printf-实现](COMPLETION_SUMMARY.md#1️⃣-assignment-1---printf-实现)
- Printf 完整实现 → [ASSIGNMENT_REPORT.md#assignment-1-printf-的实现](ASSIGNMENT_REPORT.md#assignment-1-printf-的实现)
- Printf 代码位置 → [QUICKSTART.md#assignment-1-printf-实现](QUICKSTART.md#assignment-1-printf-实现)

### ⚙️ 调度相关
- 调度算法概览 → [COMPLETION_SUMMARY.md#4️⃣-assignment-4-调度算法](COMPLETION_SUMMARY.md#4️⃣-assignment-4-调度算法)
- RR 调度实现 → [ASSIGNMENT_4_GUIDE.md#1-时间片轮转-round-robin-rr---默认算法](ASSIGNMENT_4_GUIDE.md#1-时间片轮转-round-robin-rr---默认算法)
- 优先级调度实现 → [ASSIGNMENT_4_GUIDE.md#2-优先级调度---新增算法](ASSIGNMENT_4_GUIDE.md#2-优先级调度---新增算法)
- 切换调度算法 → [ASSIGNMENT_4_GUIDE.md#切换调度算法](ASSIGNMENT_4_GUIDE.md#切换调度算法)

### 🐛 调试相关
- GDB 基础 → [ASSIGNMENT_3_GUIDE.md#2-gdb-基本命令回顾](ASSIGNMENT_3_GUIDE.md#2-gdb-基本命令回顾)
- GDB 演示 → [ASSIGNMENT_3_GUIDE.md#演示-1-观察新线程的创建和启动](ASSIGNMENT_3_GUIDE.md#演示-1-观察新线程的创建和启动)
- 高级调试 → [ASSIGNMENT_3_GUIDE.md#高级调试技巧](ASSIGNMENT_3_GUIDE.md#高级调试技巧)
- GDB 脚本 → [run/gdbinit](run/gdbinit)

### 📊 参考资料
- 项目统计 → [COMPLETION_SUMMARY.md#-实现统计](COMPLETION_SUMMARY.md#-实现统计)
- 关键函数 → [COMPLETION_SUMMARY.md#-关键函数清单](COMPLETION_SUMMARY.md#-关键函数清单)
- 编译参数 → [QUICKSTART.md#编译参数说明](QUICKSTART.md#编译参数说明)
- 内存布局 → [QUICKSTART.md#内存布局](QUICKSTART.md#内存布局)

---

## 🎬 推荐阅读顺序

### 第一次接触项目（30 分钟）
1. [QUICKSTART.md](QUICKSTART.md) - 了解项目和快速开始
2. [COMPLETION_SUMMARY.md](COMPLETION_SUMMARY.md#快速开始) - 编译和运行
3. [COMPLETION_SUMMARY.md](COMPLETION_SUMMARY.md#-四项作业完成情况) - 了解四项作业

### 深入学习（1-2 小时）
1. [ASSIGNMENT_REPORT.md](ASSIGNMENT_REPORT.md) - 详细的实现说明
2. [COMPLETION_SUMMARY.md#-核心实现要点](COMPLETION_SUMMARY.md#-核心实现要点) - 理解核心原理
3. 查看源代码 - 对照说明文档理解代码

### 实践操作（2-3 小时）
1. [ASSIGNMENT_3_GUIDE.md](ASSIGNMENT_3_GUIDE.md) - 学习如何调试
2. [ASSIGNMENT_4_GUIDE.md](ASSIGNMENT_4_GUIDE.md) - 学习调度算法
3. 修改代码 - 添加自己的线程函数或调度算法

---

## 📱 快速链接

### 源代码文件
- **线程相关**
  - 线程结构：[include/thread.h](include/thread.h)
  - 线程管理：[src/kernel/program.cpp](src/kernel/program.cpp)
  - 线程头文件：[include/program.h](include/program.h)

- **中断相关**
  - 中断处理：[src/kernel/interrupt.cpp](src/kernel/interrupt.cpp)
  - 中断头文件：[include/interrupt.h](include/interrupt.h)

- **输出相关**
  - Printf 实现：[src/kernel/stdio.cpp](src/kernel/stdio.cpp)
  - Printf 头文件：[include/stdio.h](include/stdio.h)

- **引导程序**
  - MBR：[src/boot/mbr.asm](src/boot/mbr.asm)
  - Bootloader：[src/boot/bootloader.asm](src/boot/bootloader.asm)
  - 内核入口：[src/boot/entry.asm](src/boot/entry.asm)

- **编译脚本**
  - Main Makefile：[build/makefile](build/makefile)

### 运行时文件
- 启动镜像：[run/hd.img](run/hd.img)
- GDB 配置：[run/gdbinit](run/gdbinit)

---

## 🆘 如何使用本导航

### 如果你想...

**编译和运行项目**
→ [QUICKSTART.md#最快上手](QUICKSTART.md#最快上手)

**理解 printf 的实现**
→ [ASSIGNMENT_REPORT.md#assignment-1-printf-的实现](ASSIGNMENT_REPORT.md#assignment-1-printf-的实现)

**理解线程的创建**
→ [ASSIGNMENT_REPORT.md#assignment-2-线程的实现](ASSIGNMENT_REPORT.md#assignment-2-线程的实现)

**调试线程的执行**
→ [ASSIGNMENT_3_GUIDE.md](ASSIGNMENT_3_GUIDE.md)

**学习调度算法**
→ [ASSIGNMENT_4_GUIDE.md](ASSIGNMENT_4_GUIDE.md)

**查看代码位置**
→ [QUICKSTART.md#关键代码位置](QUICKSTART.md#关键代码位置)

**解决编译问题**
→ [QUICKSTART.md#常见问题](QUICKSTART.md#常见问题)

**查看内存布局**
→ [QUICKSTART.md#内存布局](QUICKSTART.md#内存布局)

**添加新功能**
→ [ASSIGNMENT_4_GUIDE.md#添加新调度算法](ASSIGNMENT_4_GUIDE.md#添加新调度算法)

---

## 📋 文档列表

| 文档 | 大小 | 重点 | 推荐对象 |
|------|------|------|--------|
| [QUICKSTART.md](QUICKSTART.md) | 7.3K | 快速开始 | 所有人 |
| [COMPLETION_SUMMARY.md](COMPLETION_SUMMARY.md) | 13K | 项目总结 | 所有人 |
| [ASSIGNMENT_REPORT.md](ASSIGNMENT_REPORT.md) | 9.0K | 作业详解 | 学生/教师 |
| [ASSIGNMENT_3_GUIDE.md](ASSIGNMENT_3_GUIDE.md) | 7.6K | 调试演示 | 深度学习者 |
| [ASSIGNMENT_4_GUIDE.md](ASSIGNMENT_4_GUIDE.md) | 10K | 算法详解 | 深度学习者 |
| [INDEX.md](INDEX.md) | 本文档 | 导航索引 | 需要指引的人 |

---

## 🎯 最常用的三个文档

1. **[QUICKSTART.md](QUICKSTART.md)** - 快速开始，解决"怎么用"
2. **[ASSIGNMENT_REPORT.md](ASSIGNMENT_REPORT.md)** - 详细说明，解决"怎么理解"
3. **[ASSIGNMENT_3_GUIDE.md](ASSIGNMENT_3_GUIDE.md)** - 调试指南，解决"怎么调试"

---

**提示：** Markdown 文件中的链接都是可点击的，你可以直接在 IDE 中跳转。

---

最后更新：2026-05-04  
项目位置：/root/dev/study/os/lab5/
