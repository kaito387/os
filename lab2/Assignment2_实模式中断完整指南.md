# Assignment 2 - 实模式中断完整指南

> 学会用中断来控制硬件

---

## 第 1 部分：什么是中断？

### 中断的概念

中断是 CPU 响应硬件或软件请求的机制：

```
常规代码流程                          中断流程
┌─────────────┐                    ┌─────────────┐
│ 指令 A      │                    │ 指令 A      │
├─────────────┤                    ├─────────────┤
│ 指令 B      │                    │ 指令 B      │
├─────────────┤                    ├─────────────┤
│ 指令 C      │                    │ [中断发生]  │ ← 硬件或软件事件
├─────────────┤                    ├─────────────┤
│ 指令 D      │                    │ → 中断处理  │
└─────────────┘                    │ ← 返回      │
                                   ├─────────────┤
                                   │ 继续 C      │
                                   └─────────────┘
```

### 软件中断 vs 硬件中断

- **硬件中断**：键盘敲击、鼠标移动、磁盘完成读写等
- **软件中断**：由代码主动触发的中断，用 `int 中断号` 指令

我们这里用**软件中断**来调用 BIOS 的功能。

### BIOS 中断

BIOS（Basic Input Output System）提供了许多中断，每个中断通过"中断号"识别：

| 中断号 | 功能 |
|--------|------|
| 0x10 | 视频显示服务（光标、清屏等） |
| 0x13 | 磁盘服务（读写磁盘） |
| 0x16 | 键盘服务 |
| 0x21 | DOS/BIOS 综合服务 |

---

## 第 2 部分：中断调用方式

### 标准流程

```
步骤1：准备参数
      将参数写入指定寄存器（根据中断和功能号规定）

步骤2：调用中断
      int 中断号

步骤3：获取返回值
      从寄存器中读取结果
```

### 例子：设置光标位置

使用 BIOS 中断 0x10 的 02 号功能（设置光标）：

```asm
; 设置光标到 (5, 10) 位置
mov ah, 0x02           ; 功能号 02
mov bh, 0x00           ; 页码（通常为 0）
mov dh, 5              ; 行号
mov dl, 10             ; 列号
int 0x10               ; 调用视频中断
```

**参数对应关系**：
- `ah` = 功能号
- `bh` = 页码（通常 0）
- `dh` = 行号（Y 坐标）
- `dl` = 列号（X 坐标）

---

## 第 3 部分：Assignment 2.1 - 光标中断

### 任务

探索实模式下的光标中断，学会获取和设置光标位置。

### 常用功能

#### 功能 02：设置光标位置

```asm
mov ah, 0x02
mov bh, 0x00           ; 页码
mov dh, row            ; 行号（0-24）
mov dl, col            ; 列号（0-79）
int 0x10
```

#### 功能 03：获取光标位置

```asm
mov ah, 0x03
mov bh, 0x00           ; 页码
int 0x10
; 返回值在 dh(行), dl(列)
```

### 示例代码

```asm
org 0x7c00
[bits 16]

xor ax, ax
mov ds, ax
mov ss, ax
mov gs, ax
mov sp, 0x7c00

; 获取当前光标位置
mov ah, 0x03           ; 功能 03：获取光标
mov bh, 0x00
int 0x10               ; dh=行, dl=列

; 移动光标到 (10, 20)
mov ah, 0x02           ; 功能 02：设置光标
mov bh, 0x00
mov dh, 10             ; 行
mov dl, 20             ; 列
int 0x10

; 使用 print 功能输出文本
; （需要在光标位置输出）
mov al, 'H'
int 0x10               ; 0x0A 功能可以输出字符

; 死循环
jmp $
times 510 - ($ - $$) db 0
db 0x55, 0xaa
```

### 步骤

1. 创建 `assignment2_1.asm`
2. 编写代码测试光标函数
3. 编译、创建磁盘、写入、运行
4. 观察光标位置是否正确移动
5. 截图提交

---

## 第 4 部分：Assignment 2.2 - 中断输出学号

### 任务

修改 Assignment 1.2 的代码，改用中断来输出学号（而不是直接写显存）。

### 关键中断函数

#### 功能 0x0E：写字符到光标位置

```asm
mov ah, 0x0E           ; 功能 0E
mov al, 字符            ; 要输出的字符 ASCII 码
mov bh, 0x00           ; 页码
int 0x10               ; 输出字符，光标自动右移
```

### 改写步骤

1. **用中断设置光标**到 (12, 12)
2. **逐个输出**学号的每个字符（用中断）
3. 不需要计算显存地址

### 示例代码

```asm
org 0x7c00
[bits 16]

xor ax, ax
mov ds, ax
mov ss, ax
mov gs, ax
mov sp, 0x7c00

; 设置光标到 (0, 0)
mov ah, 0x02
mov bh, 0x00
mov dh, 0              ; 第 0 行
mov dl, 0              ; 第 0 列
int 0x10

; 输出 "Hello World"
mov al, 'H'
mov ah, 0x0E
mov bh, 0x00
int 0x10

mov al, 'e'
int 0x10

; ... 继续输出其他字符 ...

; 现在移动光标到 (12, 12) 输出学号
mov ah, 0x02
mov bh, 0x00
mov dh, 12             ; 行 12
mov dl, 12             ; 列 12
int 0x10

; 输出学号（假设 "2022001"）
mov al, '2'
mov ah, 0x0E
mov bh, 0x00
int 0x10

mov al, '0'
int 0x10

; ... 继续输出其他数字 ...

jmp $
times 510 - ($ - $$) db 0
db 0x55, 0xaa
```

### 相比直接写显存的优点

✅ 代码更简洁（不需要计算显存地址）  
✅ 光标自动移动  
✅ 更接近实际 OS 的做法  

---

## 第 5 部分：Assignment 2.3 - 键盘中断

### 任务

探索键盘中断，实现键盘输入并回显。

### 键盘中断

#### 中断号：0x16

| 功能 | AH 值 | 参数 | 返回值 |
|------|-------|------|--------|
| 读键 | 0x00 | 无 | al=按键 ASCII，ah=扫描码 |
| 检查键 | 0x01 | 无 | ZF=0 有键，ZF=1 无键 |

#### 功能 0x00：阻塞式读键

```asm
mov ah, 0x00           ; 功能 00
int 0x16               ; 等待键入

; al = ASCII 码
; ah = 扫描码
```

这个调用会**阻塞**（等待）直到有键按下。

### 完整的键盘输入程序

```asm
org 0x7c00
[bits 16]

xor ax, ax
mov ds, ax
mov ss, ax
mov gs, ax
mov sp, 0x7c00

; 清屏（可选）
mov al, 0x03           ; 清屏功能
mov ah, 0x00
int 0x10

; 输出提示
mov ah, 0x02
mov bh, 0x00
mov dh, 0
mov dl, 0
int 0x10

mov al, 'P'
mov ah, 0x0E
int 0x10

mov al, 'r'
int 0x10

mov al, 'e'
int 0x10

mov al, 's'
int 0x10

mov al, 's'
int 0x10

mov al, ' '
int 0x10

mov al, 'a'
int 0x10

mov al, 'n'
int 0x10

mov al, 'y'
int 0x10

mov al, ' '
int 0x10

mov al, 'k'
int 0x10

mov al, 'e'
int 0x10

mov al, 'y'
int 0x10

; 读取键盘输入
mov ah, 0x00           ; 阻塞读键
int 0x16               ; 等待按键

; al 中是按键的 ASCII 码
; 输出这个字符
mov ah, 0x0E
int 0x10               ; 显示按下的键

; 继续读取（可以在这里做循环）
jmp $
times 510 - ($ - $$) db 0
db 0x55, 0xaa
```

### 键盘扫描码

不同键有不同的扫描码（在 `ah` 中返回）：

| 键 | 扫描码 | ASCII |
|----|--------|-------|
| A | 0x1E | 0x41 |
| Enter | 0x1C | 0x0D |
| Space | 0x39 | 0x20 |
| 0-9 | 0x0B-0x0A | 0x30-0x39 |

### 完整的 Assignment 2.3：键盘循环

```asm
org 0x7c00
[bits 16]

xor ax, ax
mov ds, ax
mov ss, ax
mov gs, ax
mov sp, 0x7c00

; 移动光标到 (0, 0)
mov ah, 0x02
mov bh, 0x00
mov dh, 0
mov dl, 0
int 0x10

; 输出提示
mov al, 'T'
mov ah, 0x0E
int 0x10

mov al, 'y'
int 0x10

mov al, 'p'
int 0x10

mov al, 'e'
int 0x10

mov al, ':'
int 0x10

mov al, ' '
int 0x10

; 键盘输入循环
keyboard_loop:
    mov ah, 0x00       ; 读键
    int 0x16

    ; al 中是 ASCII 码
    cmp al, 0x0D       ; Enter 键？（ASCII 13）
    je end_loop

    ; 回显字符
    mov ah, 0x0E
    int 0x10

    jmp keyboard_loop

end_loop:
    jmp $

times 510 - ($ - $$) db 0
db 0x55, 0xaa
```

---

## 第 6 部分：常见错误

### 错误 1：参数放在错误的寄存器

```asm
❌ mov al, row             ; 错误：row 应该在 dh
   mov bl, col
   int 0x10

✅ mov dh, row             ; 行在 dh
   mov dl, col            ; 列在 dl
   int 0x10
```

### 错误 2：忘记功能号

```asm
❌ int 0x10               ; 没有 mov ah, 0x02
   ; 不知道调什么功能

✅ mov ah, 0x02           ; 设置功能号
   mov dh, 5
   mov dl, 10
   int 0x10
```

### 错误 3：光标坐标超范围

```asm
❌ mov dh, 30             ; 行号最大 24
   mov dl, 100           ; 列号最大 79
   int 0x10

✅ mov dh, 10             ; 0-24
   mov dl, 50            ; 0-79
   int 0x10
```

---

## 第 7 部分：调试技巧

### 逐步测试

```asm
; 测试 1：光标移动
mov ah, 0x02
mov bh, 0x00
mov dh, 5
mov dl, 5
int 0x10

mov al, 'X'
mov ah, 0x0E
int 0x10              ; 应该在 (5, 5) 看到 X

jmp $
```

### 使用标记字符

在关键位置用明显的字符标记，确保光标移动正确：

```asm
mov al, '+'
mov ah, 0x0E
int 0x10              ; 输出 +
; 如果 + 出现在你期望的位置，说明光标设置对了
```

---

## 总结

Assignment 2 考查的核心：

✅ 理解中断的概念  
✅ 掌握中断的调用方式  
✅ 学会使用 BIOS 中断 0x10（视频）和 0x16（键盘）  
✅ 能够设置光标、输出字符、读取键盘  
✅ 代码逻辑的循环控制  

**完成 Assignment 2 后，你就掌握了 BIOS 中断编程！** 🎉
