# Assignment 4 - 字符弹射游戏完整指南

> 写一个在 MBR 中能运行的迷你游戏

---

## 第 1 部分：需求分析

### 任务

编写一个**字符弹射程序**：
- 从屏幕位置 **(2, 0)** 开始
- 以 **45 度角向右下角** 射出
- 遇到 **边界反弹**
- 反弹后继续以 45 度角运动
- 代码 **不超过 510 字节**

### 屏幕参数

```
┌─────────────────────────────────────────┐
│ (0,0)                            (0,79) │
│                                         │
│   (2,0) ←─ 起点                         │
│        ╱                                │
│       ╱ 45度                            │
│      ╱                                  │
│     ●                                   │
│                                         │
│                                   (24,79)
└─────────────────────────────────────────┘

屏幕范围：
  行: 0 - 24（共25行）
  列: 0 - 79（共80列）
```

### 反弹规则

```
碰撞情况           反弹后的方向
──────────────────────────────
右边界(col=79)  → 向左移动
下边界(row=24)  → 向上移动
左边界(col=0)   → 向右移动
上边界(row=0)   → 向下移动

45度角运动向量：
  右下: Δrow=+1, Δcol=+1
  左下: Δrow=+1, Δcol=-1
  右上: Δrow=-1, Δcol=+1
  左上: Δrow=-1, Δcol=-1
```

---

## 第 2 部分：核心算法

### 状态变量

```asm
; 当前位置
row: 当前行 (0-24)
col: 当前列 (0-79)

; 运动方向
dir_row: 行方向 (+1=下, -1=上)
dir_col: 列方向 (+1=右, -1=左)

; 显示字符（可选变化）
char: 显示的字符（如 'O', '*', '#'）
color: 颜色（可选变化）
```

### 主循环

```
初始化：
  row = 2, col = 0
  dir_row = +1, dir_col = +1
  char = '*'

循环：
  1. 清除旧位置的字符
  2. 计算新位置 row += dir_row, col += dir_col
  3. 检查边界碰撞
     - 如果 col < 0 或 col > 79：反转 dir_col
     - 如果 row < 0 或 row > 24：反转 dir_row
  4. 在新位置显示字符
  5. 延迟（制造动画效果）
  6. 跳回循环
```

---

## 第 3 部分：代码框架（简化版）

```asm
org 0x7c00
[bits 16]

; 初始化
xor ax, ax
mov ds, ax
mov ss, ax
mov gs, ax
mov sp, 0x7c00
mov ax, 0xb800
mov gs, ax

; 初始状态
mov byte [row], 2      ; 行
mov byte [col], 0      ; 列
mov byte [dir_row], 1  ; 向下
mov byte [dir_col], 1  ; 向右

main_loop:
    ; 获取当前位置
    movzx ax, byte [row]
    movzx bx, byte [col]
    
    ; 计算显存地址 = 2*(80*row + col)
    mov cx, 80
    mul cx
    add ax, bx
    shl ax, 1           ; 乘以 2
    mov di, ax          ; di = 显存偏移
    
    ; 显示字符（蓝色 '*'）
    mov word [gs:di], 0x011E    ; 0x01=蓝色, 0x1E='*'
    
    ; 延迟（简单的忙等待）
    mov cx, 0xFFFF
delay_loop:
    loop delay_loop
    
    ; 清除当前位置
    mov word [gs:di], 0x0000    ; 清除
    
    ; 计算新位置
    movzx ax, byte [row]
    mov cl, byte [dir_row]
    movsx cx, cl        ; 符号扩展
    add ax, cx          ; 新行
    mov byte [row], al
    
    movzx ax, byte [col]
    mov cl, byte [dir_col]
    movsx cx, cl
    add ax, cx          ; 新列
    mov byte [col], al
    
    ; 检查边界（简化版，不处理反弹）
    
    jmp main_loop

; 数据段（代码末尾）
row: db 2
col: db 0
dir_row: db 1
dir_col: db 1

times 510 - ($ - $$) db 0
db 0x55, 0xaa
```

---

## 第 4 部分：完整版代码（带反弹）

```asm
org 0x7c00
[bits 16]

; 初始化
xor ax, ax
mov ds, ax
mov ss, ax
mov gs, ax
mov sp, 0x7c00

mov ax, 0xb800
mov gs, ax

; 初始状态
mov byte [row], 2      ; 开始行
mov byte [col], 0      ; 开始列
mov byte [dir_row], 1  ; 向下
mov byte [dir_col], 1  ; 向右
mov byte [char], '*'   ; 显示字符

main_loop:
    ; 计算显存地址
    movzx eax, byte [row]
    mov ecx, 80
    mul ecx
    movzx ebx, byte [col]
    add eax, ebx
    shl eax, 1
    mov edi, eax

    ; 显示字符
    mov ah, 0x01        ; 蓝色
    mov al, byte [char]
    mov [gs:edi], ax

    ; 延迟（减少字节数的做法）
    mov ecx, 0x7FFF
delay:
    loop delay

    ; 清除
    mov word [gs:edi], 0

    ; 计算新位置
    movsx eax, byte [dir_row]
    movzx ecx, byte [row]
    add eax, ecx
    
    ; 检查上下边界
    cmp eax, 0
    jge check_down
    mov eax, 0
    neg byte [dir_row]
check_down:
    cmp eax, 24
    jle row_ok
    mov eax, 24
    neg byte [dir_row]
row_ok:
    mov byte [row], al

    ; 计算新列
    movsx eax, byte [dir_col]
    movzx ecx, byte [col]
    add eax, ecx
    
    ; 检查左右边界
    cmp eax, 0
    jge check_right
    mov eax, 0
    neg byte [dir_col]
check_right:
    cmp eax, 79
    jle col_ok
    mov eax, 79
    neg byte [dir_col]
col_ok:
    mov byte [col], al

    jmp main_loop

; 数据
row: db 2
col: db 0
dir_row: db 1
dir_col: db 1
char: db '*'

times 510 - ($ - $$) db 0
db 0x55, 0xaa
```

---

## 第 5 部分：优化技巧（减小代码）

因为 MBR 只有 510 字节，需要精心优化。

### 技巧 1：减少延迟代码

```asm
; 方法 1：直接循环（占字节多）
mov ecx, 0x7FFF
delay_loop:
    loop delay_loop

; 方法 2：嵌套循环（占字节少）
mov cx, 0x1FF
loop1:
    mov bx, 0x7F
loop2:
    dec bx
    jnz loop2
    loop loop1

; 方法 3：最简洁（但可能太快）
mov cx, 0x1000
loop cx
```

### 技巧 2：共享代码

```asm
; 不要分别检查四个边界
; 用统一的比较逻辑
```

### 技巧 3：使用宏

```asm
%macro show_char 0
    mov ah, 0x01
    mov al, byte [char]
    mov [gs:edi], ax
%endmacro
```

### 技巧 4：省略不必要的初始化

```asm
; 不需要初始化所有段寄存器
; 只初始化必要的
xor ax, ax
mov ds, ax
; gs 用 mov 直接赋值
mov ax, 0xb800
mov gs, ax
```

---

## 第 6 部分：扩展功能

### 功能 1：改变颜色

```asm
; 在每次循环中改变颜色
mov al, byte [color]
inc al
cmp al, 16
jl color_ok
mov al, 0
color_ok:
mov byte [color], al
mov ah, al
```

### 功能 2：改变字符

```asm
; 轮流显示不同字符
mov al, byte [char]
inc al
cmp al, 'Z'
jle char_ok
mov al, 'A'
char_ok:
mov byte [char], al
```

### 功能 3：双球效果

维护两个位置，各自独立运动。

### 功能 4：加速/减速

修改延迟时间来改变速度。

---

## 第 7 部分：完整的精简版本

这是一个几乎达到最小代码的版本：

```asm
org 0x7c00
[bits 16]

xor ax, ax
mov ds, ax
mov gs, ax
mov ax, 0xb800
mov gs, ax

mov cl, 2       ; row
mov dl, 0       ; col
mov ah, 1       ; dir_row
mov bh, 1       ; dir_col

loop_main:
    mov al, cl
    mov bl, 80
    mul bl
    add al, dl
    mov di, ax
    shl di, 1
    
    mov byte [gs:di], '*'
    mov byte [gs:di+1], 0x01
    
    mov cx, 0x2000
    loop cx
    
    mov byte [gs:di], 0
    
    add cl, ah
    cmp cl, 0
    jge check_down
    mov cl, 0
    neg ah
check_down:
    cmp cl, 24
    jle row_ok
    mov cl, 24
    neg ah
row_ok:
    
    add dl, bh
    cmp dl, 0
    jge check_right
    mov dl, 0
    neg bh
check_right:
    cmp dl, 79
    jle col_ok
    mov dl, 79
    neg bh
col_ok:
    
    jmp loop_main

times 510 - ($ - $$) db 0
db 0x55, 0xaa
```

---

## 第 8 部分：实现步骤

### 步骤 1：创建基础版本

从简化版开始，确保基本逻辑正确。

### 步骤 2：测试运动

编译、加载、运行，观察字符是否运动。

### 步骤 3：测试反弹

检查四个边界的反弹是否正确。

### 步骤 4：优化大小

如果超过 510 字节，使用上述优化技巧。

### 步骤 5：添加特效

如果还有字节空间，添加颜色变化或其他效果。

### 编译和测试

```bash
nasm -f bin assignment4.asm -o assignment4.bin
ls -l assignment4.bin               # 检查大小

qemu-img create hd.img 10m
dd if=assignment4.bin of=hd.img bs=512 count=1 seek=0 conv=notrunc
qemu-system-i386 -hda hd.img -serial null -parallel stdio
```

---

## 第 9 部分：常见错误

### 错误 1：代码超过 510 字节

```bash
# 查看大小
nasm -f bin assignment4.asm -o assignment4.bin
wc -c assignment4.bin

# 超过 510 就要优化
```

### 错误 2：显存地址计算错误

```asm
❌ mov edi, eax        ; 直接用作显存偏移
   mov [gs:edi], ax   ; 可能寻址错误

✅ mov eax, 80
   mul [row]
   add eax, [col]
   shl eax, 1         ; 乘以 2
   mov edi, eax
   mov [gs:edi], ax
```

### 错误 3：边界检查顺序错

```asm
❌ mov eax, [row]
   add eax, [dir_row]  ; 先加后检查，可能越界

✅ mov eax, [row]
   add eax, [dir_row]
   cmp eax, 0
   jge ...
   cmp eax, 24
   jle ...
   ; 正确地约束在 0-24
```

---

## 第 10 部分：调试建议

### 快速检查

1. 字符是否显示？
2. 字符是否移动？
3. 反弹是否正确？
4. 速度是否合理？

### 逐步简化测试

```asm
; 测试 1：只显示字符，不动
mov al, '*'
mov [gs:0], al
jmp $

; 测试 2：字符移动，不反弹
; 在对角线上运动

; 测试 3：加入一个边界反弹

; 测试 4：加入四个边界反弹
```

---

## 总结

Assignment 4 考查的核心：

✅ 动态内存位置的计算  
✅ 实时更新显示  
✅ 状态管理（位置、方向）  
✅ 碰撞检测和反弹逻辑  
✅ 代码空间优化（510 字节限制）  
✅ 程序逻辑的完整性  

**完成 Assignment 4 后，你就掌握了实时交互程序的核心概念！** 🎉

---

**Tips**：这个作业很有趣，可以考虑添加各种创意效果：
- 多个球同时弹射
- 变化的颜色
- 不同的字符
- 加速/减速
- 重力效果
