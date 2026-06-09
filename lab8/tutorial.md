

# 实验八 截图教程

本教程指导你完成三个 Assignment 的代码修改、运行和 gdb 分析操作。

## 准备工作

所有代码位于 `assignments/` 目录。请按需修改：

### 切换 Assignment

编辑 `assignments/src/kernel/setup.cpp` 的 `first_thread` 函数（约第 109 行），取消对应行的注释：

```cpp
void first_thread(void *arg)
{
    printf("start process\n");

    // ★★★ 切换到当前正在做的 Assignment ★★★
    programManager.executeProcess((const char *)a1_process, 1);   // Assignment 1
    // programManager.executeProcess((const char *)a2_process, 1);   // Assignment 2
    // programManager.executeProcess((const char *)a3_process, 1);   // Assignment 3

    programManager.executeThread(second_thread, nullptr, "second", 1);
    asm_halt();
}
```

### 编译命令

```bash
cd /home/lht/dev/undergrad/os/lab8/assignments/build
make clean && make
```

### 运行命令

```bash
cd /home/lht/dev/undergrad/os/lab8/assignments/build
make run
```

### Debug 命令（使用 gdbinit 文件）

```bash
cd /home/lht/dev/undergrad/os/lab8/assignments/build

# Assignment 1
make debug                     # 先启动 qemu，再在另一个终端用 gdb 连接
# 然后在另一个终端中：
gdb -q -tui -x ../run/gdbinit_a1

# Assignment 2
gdb -q -tui -x ../run/gdbinit_a2

# Assignment 3
gdb -q -tui -x ../run/gdbinit_a3
```

> **注意**：由于 makefile 中的 `make debug` 会使用 `gnome-terminal` 打开 gdb，如果 gnome-terminal 不可用，请手动分两步执行：
>
> 终端 A：
> ```bash
> cd assignments/build
> qemu-system-i386 -S -s -hda ../run/hd.img -serial null -parallel stdio -no-reboot
> ```
>
> 终端 B：
> ```bash
> cd assignments/build
> gdb -q -tui -x ../run/gdbinit_a1    # 或 a2 / a3
> ```

---

## Assignment 1：系统调用

### 1.1 代码说明

**新增了两个自定义系统调用：**

| 系统调用号 | 函数名 | 功能 |
|-----------|--------|------|
| 6 | `factorial(n)` | 计算 n 的阶乘（内核态计算，返回结果） |
| 7 | `get_pid()` | 返回当前进程的 pid |

**代码位置：**

- `assignments/include/syscall.h` — 声明
- `assignments/src/kernel/syscall.cpp` — 用户侧包装 + 内核侧实现
- `assignments/src/kernel/setup.cpp` — 注册（第 126-129 行）+ 测试函数 `a1_process()`

**factorial 的实现思路：**

```
用户态: factorial(5)
  → asm_system_call(6, 5)     // 系统调用号=6, 参数=5
  → 5个寄存器: eax=6, ebx=5, ecx=0, edx=0, esi=0, edi=0
  → int 0x80                  // 特权级转移: CPL 3→0
  → asm_system_call_handler   // 查 system_call_table[6]
  → syscall_factorial(5)      // 内核态计算 5! = 120
  → iret                      // 返回用户态
用户态: result = 120
```

### 1.2 截图任务

#### 截图 1：系统调用执行结果

1. 确保 `setup.cpp` 中调用的是 `a1_process`
2. 编译运行：`make clean && make && make run`
3. **截图 qemu 窗口**，应显示：
   ```
   start process
   my pid: <pid>
   factorial(5) = 120
   factorial(0) = 1
   factorial(1) = 1
   ...
   factorial(8) = 40320
   halt
   ```

#### 截图 2-6：gdb 分析系统调用栈变化

使用 `gdbinit_a1`，按以下步骤操作并截图：

**截图 2 — 系统调用入口（用户栈）：**
```
(continue)
```
停在 `asm_system_call`。执行：
```
info registers eax ebx ecx edx esi edi
x/16wx $esp
```
截图 gdb 窗口，说明：eax=系统调用号，ebx=参数，$esp 是用户栈。

**截图 3 — 执行 int 0x80 前后对比：**
```
ni          # 单步执行几条指令，直到 int 0x80
```
在执行 `int 0x80` 之前截图 `info registers esp`，再 `ni` 执行 int 0x80 之后截图 `info registers esp`。两条截图对比展示栈从用户栈切换到了内核栈。

**截图 4 — 内核栈内容（特权级转移后的栈）：**
停在 `asm_system_call_handler` 开头，执行：
```
x/32wx $esp
```
截图，标注栈上的内容：SS、ESP、EFLAGS、CS、EIP、ds、es、fs、gs、pushad 保存的寄存器。

**截图 5 — TSS 内容：**
仍在 `asm_system_call_handler` 中，执行：
```
p/x tss
```
或：
```
p/x tss.ss0
p/x tss.esp0
```
截图，说明 ss0 和 esp0 在特权级转移中的作用——CPU 从 TSS 中读取内核栈地址并加载到 SS:ESP。

**截图 6 — 系统调用返回：**
多次 `continue` 直到 factorial 的调用返回到 `asm_system_call` 中的 `ret` 指令处。执行：
```
info registers eax
p $cs
```
截图，说明 eax 中为 factorial 的计算结果，系统已返回用户态。

### 1.3 分析要点（需要写进报告）

- `int 0x80` 执行时发生了什么？为什么 esp 值变化了？
- TSS 中的 `ss0` 和 `esp0` 什么时候被更新？谁更新的？（提示：`activateProgramPage`）
- 为什么系统调用参数必须通过寄存器而不是栈传递？

---

## Assignment 2：Fork 的奥秘

### 2.1 代码说明

使用 `a2_process`（Stage 6 原有的 fork/exit/wait 综合测试）。

代码逻辑：
```
a2_process():
  pid = fork()       // 创建第一个子进程
  if (pid) {
    pid2 = fork()    // 再创建第二个子进程
    if (pid2) {
      while(wait())  // 父进程等待回收所有子进程
      halt
    } else {
      delay(); exit(123934)  // 第二个子进程退出
    }
  } else {
    delay(); exit(-123)      // 第一个子进程退出
  }
```

### 2.2 截图任务

#### 截图 1：Fork 运行结果

1. 确保 `setup.cpp` 中调用的是 `a2_process`
2. 编译运行：`make clean && make && make run`
3. **截图 qemu 窗口**，应显示父子进程的 fork 返回值和 wait 回收信息。

#### 截图 2-8：gdb 分析 fork 执行流程

使用 `gdbinit_a2`，按下述步骤操作。

**截图 2 — 用户代码调用 fork 的位置（eax 不是 2！）：**
```
(continue)
```
停在 `fork`。**重要说明：** `fork` 是一个包装函数 `int fork() { return asm_system_call(2); }`。断点停在函数入口处时，**eax 尚未被设置为 2**，保存的是上层代码的残留值（通常为 0，来自前一条语句的返回值）。系统调用号 `2` 是 `asm_system_call` 的调用参数，只有在执行 `asm_system_call` 内部的 `mov eax, [ebp+2*4]` 后才会加载到 eax 中。
```
fork_watch
```
截图 gdb 窗口，说明：CS=0x2b（用户态），ESP 为用户栈地址，返回地址指向 a2_process 中调用 fork 的位置。

**截图 3 — copyProcess 中的 eax=0：**
```
(continue)  # 到 ProgramManager::fork
(continue)  # 到 copyProcess
```
执行 `ni` 逐步走到 `childpps->eax = 0` 这一行。
截图，标注：子进程的 ProcessStartStack.eax 被赋值为 0，这是子进程 fork 返回 0 的关键。

**截图 4 — 子进程首次被调度——asm_start_process：**
```
(continue)  # 多次，直到进入 asm_start_process
```
停在 `asm_start_process` 开头。此时是子进程第一次被调度执行。
截图：
```
info registers esp
```

**截图 5 — 子进程 popad 后 eax=0：**
```
ni          # mov esp, eax (esp 指向 ProcessStartStack)
ni          # popad (恢复通用寄存器)
info registers eax
```
截图 `info registers eax`，eax 应为 0。说明子进程中 fork 返回值为 0 的原因。

**截图 6 — 子进程段寄存器恢复：**
```
ni          # pop gs
ni          # pop fs
ni          # pop es
ni          # pop ds
info registers ds es fs gs
```
截图，说明段寄存器被设置为用户数据段选择子（DPL=3）。

**截图 7 — 子进程 iret 进入用户态：**
```
ni          # iret
info registers cs eip
```
截图，说明 cs 为用户代码段选择子（RPL=3），eip 为 a2_process 中的返回地址。

**截图 8 — 比较父子进程的 fork 返回值：**

子进程 continue 后会打印 `"exit, pid: <child_pid>"`。截图最终的 qemu 输出，把父子进程的 fork 返回值用红框标出：父进程返回值=子进程 pid，子进程返回值=0。

### 2.3 分析要点（需要写进报告）

- 子进程从第一次调度到从 fork 返回的完整跳转路径是什么？
- 为什么 `childpps->eax = 0` 保证了子进程 fork 返回 0？
- 父进程的 fork 返回值是从哪条路径得到的？（提示：`ProgramManager::fork()` return pid → `syscall_fork` → 存入 `ASM_TEMP` → `mov eax, [ASM_TEMP]`）

---

## Assignment 3：哼哈二将 wait & exit

### 3.1 代码说明

**新增了僵尸进程回收机制**（在 `assignments/src/kernel/program.cpp` 的 `schedule()` 函数中）：

原代码只在 schedule 中回收 DEAD 线程，DEAD 进程留待父进程 wait 回收。如果父进程先于子进程退出（子进程成为孤儿进程），子进程退出后无人回收，成为永久僵尸。

**修改的 schedule 逻辑：**
- 当 running 状态为 DEAD 时，若它是进程（pageDirectoryAddress != 0），检查父进程是否存活
- 若父进程不存在或状态为 DEAD，则直接回收此僵尸进程（打印 "recycle zombie process, pid: XX"）
- 若父进程存活，保留 PCB 等待父进程的 wait 回收

**a3_process 测试场景：**
```
父进程 fork → 两个子进程
  父进程: 不调用 wait，直接 exit(0)
  子进程1: 延时退出 exit(123)，成为孤儿进程 → 退出后被 schedule 自动回收
  子进程2: 延时退出 exit(456)，成为孤儿进程 → 退出后被 schedule 自动回收
```

### 3.2 截图任务

#### 截图 1：僵尸进程回收运行结果 (QEMU)

1. 确保 `setup.cpp` 中调用的是 `a3_process`
2. 编译运行：`make clean && make && make run`
3. **截图 qemu 窗口**，应显示：
   ```
   I am parent, pid: 3, my child pid: 4
   parent exit without waiting, pid: 3
   first child (orphan), pid: 4
   recycle zombie process, pid: 4
   thread exit
   halt
   second child exit, pid: 5
   recycle zombie process, pid: 5
   halt
   ```
   关键：看到 `recycle zombie process` 输出，证明僵尸进程被自动回收。

#### 截图 2-4：gdb 分析 exit（用 a3_process）

确保 `setup.cpp` 调用的是 `a3_process`。使用 `gdbinit_a3`。

**截图 2 — exit 入口：查看谁在退出、携带什么返回值**

```
(continue)
```
停在 `ProgramManager::exit`（BP1）。依次执行以下命令并截图：

```gdb
# 谁在退出？
p programManager.running->pid
# 返回值是多少？
p ret
# 当前是进程还是线程？（非 0 = 进程，0 = 线程）
p/x programManager.running->pageDirectoryAddress
# 全局状态
process_watch
```

**截图 3 — exit 标记 DEAD 后、释放页表过程中**

```gdb
# 先 ni 几步，走过 "program->status = ProgramStatus::DEAD" 这一行
# 确认状态已变为 DEAD（2）
p programManager.running->status

# ni 进入 for 循环释放物理页的过程。
# 在 releasePhysicalPages 调用处停下，查看正在释放的物理页地址：
p paddr

# 继续 ni 到释放页目录表的 releasePages 调用处，查看页目录表地址：
p/x pageDir
```

截图 gdb 窗口，包含上述关键变量的输出。

**截图 4 — exit 末尾，调用 schedule 之前**

在 exit 函数末尾，所有资源释放完成后、`schedule()` 调用之前：

```gdb
# 确认退出进程的资源已全部释放
p programManager.running->pid
p programManager.running->status
p programManager.allPrograms.size()
p programManager.readyPrograms.size()
```

截图 gdb 窗口。

#### 截图 5-6：gdb 分析 wait（用 a2_process）

> **重要：切换到 a2_process！** 因为 a3_process 中父进程不调用 wait。
> 编辑 `setup.cpp`，把 `first_thread` 中的调用改为 `a2_process`，然后 `make clean && make`。

重新启动 debug，使用 `gdbinit_a3`。

**截图 5 — wait 找到 DEAD 子进程**

首先理解执行时序：父进程第一次调用 `wait` 时，两个子进程还在执行 `exit` 之前的 delay 循环（`uint32 tmp = 0xffffff; while(tmp) --tmp;`），状态都是 RUNNING，尚未 DEAD。wait 发现"有子进程但未死"后会调用 `schedule()` 让出 CPU，父进程被阻塞。

所以你需要多次 `continue`，让子进程跑完 delay → 调用 exit → 变成 DEAD。步骤如下：

```gdb
(continue)
```
第一次停在 `ProgramManager::wait`（BP2）。执行：

```gdb
# 查看当前上下文
process_watch

# 遍历链表看子进程状态——此时应该都是 RUNNING/READY，没有 DEAD
p child->pid
p child->status       # 预计 = 1 (RUNNING) 或 3 (READY)，不会是 2 (DEAD)
```

此时 wait 找不到 DEAD 子进程，会调用 `schedule()` 阻塞。继续：

```gdb
(continue)   # wait 内部调 schedule，切到子进程执行
(continue)   # 子进程 exit，可能停在 BP1（ProgramManager::exit）
(continue)   # 继续
...
# 反复 continue，直到再次停在 ProgramManager::wait（BP2）。
# 这次子进程已经退出（DEAD），wait 能找到它了。
```

当 wait 遍历到 DEAD 子进程时：

```gdb
# 查看找到的 DEAD 子进程
p child->pid
p child->status       # 应为 2 (DEAD)
p child->retValue     # 子进程的退出返回值
p child->parentPid    # 应等于父进程 pid
```

截图 gdb 窗口。

```gdb
# 查看即将被回收的子进程信息
p child->pid
p child->retValue
p *retval

# ni 执行 releasePCB
# 再查看 allPrograms 确认 PCB 被移除
p programManager.allPrograms.size()
```

截图 gdb 窗口。

#### 截图 7：隐式 exit 分析（load_process 中的栈设置）

> 可以用 a1_process 或 a2_process，不限定。

```gdb
b _Z12load_processPKc
(continue)
```

停在 `load_process`。多次 `ni` 逐步执行，直到图中类似以下代码（约在函数末尾，`asm_start_process` 调用之前）：

```c
userStack[0] = (int)exit;
userStack[1] = 0;
userStack[2] = 0;
```

走到 `userStack[2] = 0;` 之后，执行：

```gdb
# 查看 3 特权级栈顶放的三个值：
p/x userStack[0]
p/x userStack[1]
p/x userStack[2]
# 验证 userStack[0] 确实是 exit 的地址
p/x exit
```

截图 gdb 窗口，说明：进程主函数返回时会从栈弹出"返回地址" = `exit` 的地址，`userStack[1]` 被当作 exit 的"返回地址"（=0，无意义），`userStack[2]` 被当作 exit 的参数（ret=0）。因此进程返回后隐式调用了 `exit(0)`。

### 3.3 分析要点（需要写进报告）

- exit 释放了哪些资源？释放顺序是什么？
- 为什么进程退出后能隐式调用 exit 且返回值为 0？
- wait 在什么情况下会阻塞？什么情况下返回 -1？
- 僵尸进程产生的原因是什么？你的修改是如何解决这个问题的？

---

## 截图清单汇总

### Assignment 1 需要截图：
| # | 内容 |
|---|------|
| 1 | qemu 运行结果（factorial 系统调用输出） |
| 2 | gdb: asm_system_call 入口（寄存器+栈） |
| 3 | gdb: int 0x80 前后 esp 对比 |
| 4 | gdb: asm_system_call_handler 的内核栈内容 |
| 5 | gdb: TSS 结构（ss0/esp0） |
| 6 | gdb: 系统调用返回后的 eax 和 cs |

### Assignment 2 需要截图：
| # | 内容 |
|---|------|
| 1 | qemu 运行结果（fork 输出） |
| 2 | gdb: 父进程进入 fork（eax=2） |
| 3 | gdb: copyProcess 中 childpps->eax = 0 |
| 4 | gdb: 子进程 asm_start_process 入口 |
| 5 | gdb: 子进程 popad 后 eax=0 |
| 6 | gdb: 子进程 段寄存器恢复（DPL=3） |
| 7 | gdb: 子进程 iret 后 cs/eip |
| 8 | qemu 输出中父子进程 fork 返回值对比 |

### Assignment 3 需要截图：
| # | 内容 |
|---|------|
| 1 | qemu 运行结果（僵尸进程回收输出） |
| 2 | gdb: exit 入口 + process_watch |
| 3 | gdb: exit 释放页表过程 |
| 4 | gdb: exit 调用 schedule 前 |
| 5 | gdb: wait 遍历链表查找子进程 |
| 6 | gdb: wait 回收子进程 PCB |
| 7 | gdb: load_process 中隐式 exit 的栈设置 |

---

## 提示

1. gdb 中 `ni` = next instruction（单步跳过函数调用），`si` = step instruction（单步进入函数调用）
2. gdb 中 `continue` 或 `c` = 继续执行直到下一个断点
3. gdb 中 `info registers <reg>` = 查看寄存器
4. gdb 中 `x/Nwx $esp` = 以十六进制查看栈上 N 个 word
5. gdb 中 `p/x <variable>` = 以十六进制打印变量
6. gdb 中可以按 Ctrl+L 清屏
7. TUI 模式下 `Ctrl+X+A` 切换 UI 模式
8. 如果不小心 `continue` 过了断点，可以 Ctrl+C 中断，然后 `info registers` 查看当前状态
