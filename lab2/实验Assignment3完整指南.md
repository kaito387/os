# Assignment 3 完整步骤指南

> 一步步带你完成 Assignment 3 的三个小题

---

## 第 0 步：环境检查

### 安装必要的工具

```bash
sudo apt update
sudo apt install gcc-multilib g++-multilib nasm
```

如果已安装，可以验证：

```bash
nasm --version       # 查看 NASM 版本
g++ --version        # 查看 g++ 版本
which gdb            # 检查 gdb 是否存在（用于调试）
```

### 查看文件结构

```bash
cd /home/lht/dev/study/os/lab2/materials/assignment
ls -la
```

你会看到：
```
head.include        # ← 汇编头部
student.asm         # ← 你要编辑的文件
end.include         # ← 汇编尾部
test.cpp            # ← C++ 测试（不要改）
makefile            # ← 编译脚本
```

---

## 第 1 步：理解框架代码

### 打开 student.asm

```bash
cat student.asm
```

你会看到：

```asm
%include "head.include"
; you code here

your_if:
; put your implementation here

your_while:
; put your implementation here

%include "end.include"

your_function:
; put your implementation here
```

### 打开 head.include（查看声明）

```bash
cat head.include
```

```asm
global student_function
global your_function

extern a1
extern a2
extern my_random
extern your_string
extern if_flag
extern while_flag
extern print_a_char
```

**这意味着**：
- `a1, a2, if_flag, while_flag` 是来自 C++ 的全局变量
- `your_string, my_random, print_a_char` 是 C++ 函数
- 你要实现的函数 `your_function` 会被 C++ 调用

### 打开 test.cpp（理解测试逻辑）

```bash
head -50 test.cpp
```

关键部分：

```cpp
extern "C" void your_function();
extern "C" void student_function();

int a1, a2;
const char *your_string = "Mr.Chen, students and TAs are the best!\n";
char *while_flag, *random_buffer;

void student_setting() {
    a1 = 24;      // ← 你可以改这个来测试
    a2 = 14;      // ← 这个也可以改
}
```

---

## 第 2 步：做一次完整的编译和运行

### 编译并运行

```bash
cd /path/to/materials/assignment
make run
```

你会看到输出（即使 `.asm` 文件是空的也能编译，因为还有框架）。

这验证了：
✅ 编译工具正确安装  
✅ Makefile 正确  
✅ 可以进行开发  

---

## 第 3 步：Assignment 3.1 - 分支逻辑

### 需求回顾

```cpp
if (a1 < 12) {
    if_flag = a1 / 2 + 1;
} else if (a1 < 24) {
    if_flag = (24 - a1) * a1;
} else {
    if_flag = a1 << 4;
}
```

### 第 1 个测试用例：a1 = 10

**计算期望结果**：

- 10 < 12 ✓ 进入第一个分支
- if_flag = 10 / 2 + 1 = 5 + 1 = **6**

### 编写汇编代码

打开 `student.asm`，替换 `your_if:` 部分：

```asm
your_if:
    ; 第一个条件：a1 < 12
    mov eax, dword [a1]      ; eax = a1
    cmp eax, 12              ; 比较 eax 和 12
    jge elif_2               ; 如果 >= 12，跳到第二个条件

    ; 分支 1：if_flag = a1/2 + 1
    mov eax, dword [a1]      ; eax = a1
    mov ecx, 2               ; ecx = 2
    cdq                      ; 符号扩展
    idiv ecx                 ; eax = a1 / 2
    add eax, 1               ; eax = (a1/2) + 1
    mov dword [if_flag], eax ; 存储结果
    jmp if_done              ; 跳到结束

elif_2:
    ; 第二个条件：a1 < 24
    mov eax, dword [a1]
    cmp eax, 24
    jge else_3

    ; 分支 2：if_flag = (24 - a1) * a1
    mov eax, 24              ; eax = 24
    mov ecx, dword [a1]      ; ecx = a1
    sub eax, ecx             ; eax = 24 - a1
    imul eax, ecx            ; eax = (24-a1) * a1
    mov dword [if_flag], eax
    jmp if_done

else_3:
    ; 分支 3：if_flag = a1 << 4
    mov eax, dword [a1]
    shl eax, 4               ; 左移 4 位
    mov dword [if_flag], eax

if_done:
    ; 分支完毕
```

### 测试

```bash
cd materials/assignment
make run
```

### 验证结果

运行后应该打印测试结果。如果正确，会显示 pass。

### 验证所有测试用例

修改 `test.cpp` 中的 `student_setting` 来测试不同的 `a1` 值：

**测试用例**：

| a1 值 | 进入分支 | 期望 if_flag |
|-------|--------|------------|
| 10 | 第 1 个 | 6 |
| 20 | 第 2 个 | 80 |
| 30 | 第 3 个 | 480 |

```cpp
void student_setting() {
    a1 = 10;  // 改这个
    a2 = 14;
}
```

每改一次，`make run` 一次验证。

---

## 第 4 步：Assignment 3.2 - 循环逻辑

### 需求回顾

```cpp
while (a2 >= 12) {
    call my_random();           // 返回值在 eax
    while_flag[a2 - 12] = eax;
    --a2;
}
```

### 理解 my_random()

- 它是一个 C++ 函数，会返回一个随机字符到 `eax`
- 每次调用都产生不同的结果
- 它还会填充 `random_buffer` 数组（test.cpp 中定义）

### 第 1 个测试用例：a2 = 14

**预期行为**：

```
循环 1: a2=14 >= 12 ✓ → my_random() → while_flag[2] = 结果，a2--
循环 2: a2=13 >= 12 ✓ → my_random() → while_flag[1] = 结果，a2--
循环 3: a2=12 >= 12 ✓ → my_random() → while_flag[0] = 结果，a2--
循环 4: a2=11 >= 12 ✗ 退出循环
```

所以 `while_flag[0]`, `while_flag[1]`, `while_flag[2]` 会被填充。

### 编写汇编代码

```asm
your_while:
    mov ecx, dword [a2]      ; ecx = a2（用作循环计数器）

while_loop:
    cmp ecx, 12              ; 检查 a2 >= 12 ?
    jl while_end             ; 如果 < 12，退出循环

    ; 调用 my_random()
    pushad                   ; 保存寄存器（因为 my_random 可能修改）
    call my_random           ; 返回值在 eax
    popad                    ; 恢复寄存器（除了 eax）

    ; ⚠️ 问题：popad 会恢复 eax！
    ; 解决：把返回值保存到栈或其他寄存器
    
    ; 正确做法：
    ; ...
    popad
    call my_random
    ; 现在 eax 是返回值
    
    ; 计算数组下标：a2 - 12
    mov ebx, ecx             ; ebx = a2
    sub ebx, 12              ; ebx = a2 - 12
    
    ; 存储到 while_flag[a2 - 12]
    mov byte [while_flag + ebx], al  ; al 是返回字符
    
    ; --a2（即 ecx--）
    dec ecx
    jmp while_loop

while_end:
    ; 循环完毕
    mov dword [a2], ecx      ; 更新 a2 的值（可选，但保险）
```

**问题**：`popad` 会覆盖 `eax`！

**解决方案**：

```asm
your_while:
    mov ecx, dword [a2]

while_loop:
    cmp ecx, 12
    jl while_end

    ; 方法 1：不用 pushad/popad（如果 my_random 不修改其他寄存器）
    call my_random           ; eax = 返回值
    
    mov ebx, ecx
    sub ebx, 12
    mov byte [while_flag + ebx], al
    
    ; 方法 2：手动保存需要的寄存器
    push ecx                 ; 保存 ecx
    push ebx                 ; 保存 ebx
    call my_random           ; eax = 返回值
    pop ebx                  ; 恢复 ebx
    pop ecx                  ; 恢复 ecx
    
    mov ebx, ecx
    sub ebx, 12
    mov byte [while_flag + ebx], al
    
    dec ecx
    jmp while_loop

while_end:
    mov dword [a2], ecx
```

### 测试

```bash
make run
```

---

## 第 5 步：Assignment 3.3 - 函数调用

### 需求回顾

```cpp
void your_function() {
    for (int i = 0; your_string[i] != '\0'; ++i) {
        print_a_char(your_string[i]);
    }
}
```

其中 `your_string = "Mr.Chen, students and TAs are the best!\n"`

### 关键点

1. `your_string` 是一个 **字符串指针**（指向字符数组首地址）
2. 字符串以 `\0` 结尾
3. `print_a_char()` 接收一个字符参数

### 编写汇编代码

```asm
your_function:
    push ebp
    mov ebp, esp

    xor ecx, ecx             ; ecx = 0（计数器 i）
    mov esi, [your_string]   ; esi = 字符串首地址

loop_start:
    mov al, byte [esi + ecx] ; al = your_string[i]
    cmp al, 0                ; 是否为 '\0' ?
    je loop_end              ; 是的话，退出循环

    ; 保存寄存器（可能被 print_a_char 修改）
    pushad

    ; 把字符作为参数传递给 print_a_char
    movzx eax, al            ; 扩展 al 到 eax（确保传整数）
    push eax                 ; 参数入栈

    ; 调用 print_a_char
    call print_a_char

    ; 清理栈（1 个参数 × 4 字节）
    add esp, 4

    ; 恢复寄存器
    popad

    inc ecx                  ; ++i
    jmp loop_start

loop_end:
    pop ebp
    ret
```

### 测试

```bash
make run
```

应该输出：`Mr.Chen, students and TAs are the best!`

---

## 常见错误和解决

### 错误 1：Segmentation fault

**原因**：访问了非法内存地址

**检查**：
- [ ] 指针是否正确初始化？
- [ ] 数组下标是否越界？
- [ ] 地址计算是否正确？

**调试**：加入打印语句

```asm
; 在关键位置打印值
mov al, [eax]
push eax
call print_a_char
pop eax
```

### 错误 2：Wrong result

**原因**：逻辑错误

**检查清单**：
- [ ] 运算顺序对吗？
- [ ] 有没有用对寄存器？
- [ ] cmp/jmp 的逻辑对吗？

**调试方法**：手工模拟代码执行

```
初始：a1 = 10
mov eax, [a1]      → eax = 10
cmp eax, 12        → ZF=0, CF=1（10 < 12）
jge elif_2         → 不跳（因为 >= 条件不满足）
...
```

### 错误 3：Undefined reference

**原因**：缺少 `extern` 声明

**检查**：
```asm
extern a1           ; ✓ 必须有
extern if_flag      ; ✓ 必须有
extern print_a_char ; ✓ 必须有
```

### 错误 4：Stack corruption

**原因**：push/pop 不匹配

**检查**：
```bash
grep -c "push" student.asm
grep -c "pop" student.asm
```

两个数字应该相等。

---

## 快速检查清单

完成每个小题后：

- [ ] 代码能编译吗？（`make run` 无错误）
- [ ] 逻辑正确吗？（手工验证几个测试用例）
- [ ] 没有 segfault 吗？
- [ ] 结果符合预期吗？
- [ ] 代码有注释吗？（便于理解）
- [ ] 使用了正确的汇编语法吗？

---

## 进阶技巧

### 技巧 1：用计算器验证数学

```python
# 打开 Python 检查你的期望结果
python3
>>> a1 = 20
>>> a1 // 2 + 1      # 分支 1
11
>>> (24 - a1) * a1   # 分支 2
80
>>> a1 << 4           # 分支 3（左移 4 位）
320
```

### 技巧 2：用 printf 添加调试输出

在 `test.cpp` 的 `student_setting` 后添加：

```cpp
void debug_print() {
    printf("a1 = %d, a2 = %d\n", a1, a2);
    printf("if_flag = %d\n", if_flag);
}
```

然后在 `main` 中调用这个函数。

### 技巧 3：逐个禁用代码段

如果不确定哪部分有问题，注释掉一部分：

```asm
; your_while:
    ; mov ecx, dword [a2]
    ; while_loop:
    ; ...
; 临时禁用 while
```

然后只测试其他部分。

---

## 最后：提交前检查

```bash
cd materials/assignment

# 1. 检查代码能编译
make clean
make run

# 2. 用不同的 a1, a2 值测试
# 修改 test.cpp 中的 student_setting
# 重新 make run 验证

# 3. 清理编译产物
make clean

# 4. 最终截图
make run
# 截图保存结果
```

---

**祝你成功！** 🎉
