# Assignment 1 - MBR 完整指南

> 从零开始写你的第一个操作系统启动程序

---

## 第 1 部分：基本概念

### 什么是 MBR？

```
┌──────────────────────────────────────┐
│ 计算机启动过程                       │
├──────────────────────────────────────┤
│ 1. 加电                              │
│ 2. BIOS 自检                         │
│ 3. BIOS 加载硬盘首扇区(512字节)      │ ← MBR
│ 4. CPU 从 0x7C00 开始执行 MBR        │
│ 5. MBR 加载 Bootloader               │
│ 6. Bootloader 加载内核               │
│ 7. 内核启动                          │
└──────────────────────────────────────┘
```

**MBR** = Master Boot Record（主启动扇区）
- 位置：硬盘**首扇区**（第一个 512 字节）
- 大小：恰好 **512 字节**
- 加载地址：内存 **0x7C00**
- 标记：最后 2 字节必须是 **0x55, 0xAA**（启动标志）

### 显存原理

显示屏是个 **25 × 80** 的字符矩阵：

```
行  0: ████████████████████... (80 个字符)
行  1: ████████████████████...
...
行 24: ████████████████████...
```

**每个字符占 2 字节**：
```
高字节（颜色属性）: [背景色(4位)|前景色(4位)]
低字节（字符）    : [ASCII码]
```

**颜色值**：
- 0x0 = 黑色
- 0x1 = 蓝色
- 0x2 = 绿色
- 0x3 = 青色
- 0x4 = 红色
- 0x5 = 紫色
- 0x6 = 黄色
- 0x7 = 白色

### 显存地址计算

```
显存首地址: 0xB8000

对于屏幕位置 (行, 列) = (x, y)
显存地址 = 0xB8000 + 2 × (80 × x + y)
```

**例子**：在第 5 行第 10 列显示字符
```
地址 = 0xB8000 + 2 × (80 × 5 + 10)
     = 0xB8000 + 2 × 410
     = 0xB8000 + 820
     = 0xB8334
```

---

## 第 2 部分：16 位汇编基础（实模式）

### 关键寄存器

| 寄存器 | 用途 | 特殊说明 |
|--------|------|---------|
| **ax** | 累加器 | 一般用途 |
| **bx** | 基址 | 内存寻址 |
| **cx** | 计数器 | loop 用 |
| **dx** | 数据 | 一般用途 |
| **sp** | 栈指针 | ⚠️ 不随意改 |
| **bp** | 帧指针 | 通常不用 |
| **gs** | 段寄存器 | 显存段地址 |

### 段地址 + 偏移地址

16 位实模式中，物理地址的计算：

```
物理地址 = (段寄存器 << 4) + 偏移地址
```

**例子**：
```
gs = 0xB800, 偏移 = 0x0002
物理地址 = (0xB800 << 4) + 0x0002
        = 0xB8000 + 0x0002
        = 0xB8002
```

**内存寻址语法**：
```asm
mov [gs:偏移], 值      ; 通过 gs 段寄存器访问
mov [ds:偏移], 值      ; 通过 ds 段寄存器访问
```

### 常用指令（16 位）

```asm
; 数据移动
mov ax, 10             ; ax = 10
mov bx, ax             ; bx = ax
mov [gs:2], al         ; 内存 = al

; 算术
add ax, 5              ; ax += 5
sub ax, 3              ; ax -= 3
inc ax                 ; ax++
dec ax                 ; ax--

; 逻辑
shl ax, 4              ; ax <<= 4（左移）
shr ax, 2              ; ax >>= 2（右移）

; 比较和跳转
cmp ax, 10             ; 比较
je label               ; 相等跳转
jl label               ; 小于跳转
jmp label              ; 无条件跳转

; 循环
loop label             ; cx--，如果 cx != 0 则跳转
```

---

## 第 3 部分：Example 1 详解

PDF 中提供的 Example 1 代码：

```asm
org 0x7c00             ; 代码从 0x7c00 开始
[bits 16]              ; 16 位代码格式

; 初始化段寄存器
xor ax, ax             ; ax = 0
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

; 初始化栈指针
mov sp, 0x7c00
mov ax, 0xb800
mov gs, ax             ; gs = 0xB800（显存段地址）

; 输出 "Hello World"，颜色是蓝色(0x01)
mov ah, 0x01           ; 颜色属性：蓝色
mov al, 'H'            ; 字符
mov [gs:2 * 0], ax     ; 放到显存

mov al, 'e'
mov [gs:2 * 1], ax

mov al, 'l'
mov [gs:2 * 2], ax

mov al, 'l'
mov [gs:2 * 3], ax

mov al, 'o'
mov [gs:2 * 4], ax

; ... 继续输出其他字符

mov al, 'd'
mov [gs:2 * 10], ax

jmp $                  ; 死循环

; 填充到 510 字节（最后 2 字节留给启动标志）
times 510 - ($ - $$) db 0

; 启动标志
db 0x55, 0xaa
```

### 关键点解释

| 代码 | 解释 |
|------|------|
| `org 0x7c00` | 标号地址从 0x7c00 开始计算 |
| `[bits 16]` | 按 16 位格式编译 |
| `xor ax, ax` | 清零 ax（ax = 0）|
| `mov gs, ax` | 不能直接对段寄存器赋值，要通过 ax |
| `mov gs, 0xB800` | ❌ 错误，必须用寄存器中转 |
| `[gs:2*i]` | 显存地址 = gs×16 + 2×i |
| `mov ah, 0x01` | ah = 颜色属性（蓝色） |
| `mov al, 'H'` | al = 字符（'H' = 0x48） |
| `mov [gs:2*0], ax` | ax 的内容写到显存 |
| `jmp $` | 死循环（$=当前地址） |
| `times 510-($ - $$) db 0` | 填充 0 直到第 510 字节 |
| `db 0x55, 0xaa` | 启动标志，告诉 BIOS 这是可启动的 |

---

## 第 4 部分：完整步骤

### 步骤 1：了解文件位置

```bash
cd materials/src
ls -la
# 应该能看到 mbr.asm 和 hd.img
```

### 步骤 2：复现 Example 1

打开 `materials/uefi/example1.md` 中的 MBR 代码，或从 PDF 中复制 Example 1。

创建文件 `hello_mbr.asm`：

```asm
org 0x7c00
[bits 16]

; 初始化
xor ax, ax
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

mov sp, 0x7c00
mov ax, 0xb800
mov gs, ax

; 输出 Hello World
mov ah, 0x01           ; 蓝色
mov al, 'H'
mov [gs:0], ax
mov al, 'e'
mov [gs:2], ax
mov al, 'l'
mov [gs:4], ax
mov al, 'l'
mov [gs:6], ax
mov al, 'o'
mov [gs:8], ax
mov al, ' '
mov [gs:10], ax
mov al, 'W'
mov [gs:12], ax
mov al, 'o'
mov [gs:14], ax
mov al, 'r'
mov [gs:16], ax
mov al, 'l'
mov [gs:18], ax
mov al, 'd'
mov [gs:20], ax

jmp $
times 510 - ($ - $$) db 0
db 0x55, 0xaa
```

### 步骤 3：编译

```bash
nasm -f bin hello_mbr.asm -o hello_mbr.bin
```

### 步骤 4：创建虚拟磁盘

```bash
qemu-img create hd_test.img 10m
```

### 步骤 5：写入 MBR

```bash
dd if=hello_mbr.bin of=hd_test.img bs=512 count=1 seek=0 conv=notrunc
```

### 步骤 6：用 QEMU 运行

```bash
qemu-system-i386 -hda hd_test.img -serial null -parallel stdio
```

你应该会看到屏幕第一行显示蓝色的 "Hello World"！

---

## 第 5 部分：Assignment 1.1 - 复现 Example 1

**任务**：成功运行 Example 1 的 MBR 程序

### 操作步骤

1. 在 `materials/src/` 目录下或任意目录创建 `mbr.asm`
2. 复制 PDF 中的 Example 1 代码
3. 编译：`nasm -f bin mbr.asm -o mbr.bin`
4. 创建磁盘：`qemu-img create hd.img 10m`
5. 写入 MBR：`dd if=mbr.bin of=hd.img bs=512 count=1 seek=0 conv=notrunc`
6. 运行：`qemu-system-i386 -hda hd.img -serial null -parallel stdio`

### 验证

运行后应该看到：
- 屏幕第一行显示 "Hello World"
- 字符是蓝色（前景色）且背景是黑色

### 截图提交

截图 QEMU 窗口，显示 "Hello World"

---

## 第 6 部分：Assignment 1.2 - 输出学号

**任务**：修改 Example 1，在 (12, 12) 位置输出你的学号，颜色不同

### 修改要点

1. **计算显存地址**：
```
位置 (12, 12) 对应显存地址
= 0xB8000 + 2 × (80 × 12 + 12)
= 0xB8000 + 2 × (960 + 12)
= 0xB8000 + 2 × 972
= 0xB8000 + 1944
= 0xB8798

偏移地址 = 1944 = 0x798
```

2. **选择不同的颜色**（改变 `ah` 的值）：
```
ah 可选值：
0x00 = 黑底黑字（看不见！）
0x04 = 黑底红字
0x05 = 黑底紫字
0x06 = 黑底黄字
0x70 = 白底黑字
0xF0 = 白底白字（看不见！）
```

3. **示例代码**：

假设你的学号是 "2022001"，要在 (12, 12) 输出红色字：

```asm
; 移动到位置 (12, 12)
; 需要计算偏移量
mov cx, 1944           ; 偏移 = 1944 字节

; 输出学号
mov ah, 0x04           ; 红色
mov al, '2'
mov [gs:cx], ax
add cx, 2              ; 下一个字符

mov al, '0'
mov [gs:cx], ax
add cx, 2

mov al, '2'
mov [gs:cx], ax
add cx, 2

mov al, '2'
mov [gs:cx], ax
add cx, 2

mov al, '0'
mov [gs:cx], ax
add cx, 2

mov al, '0'
mov [gs:cx], ax
add cx, 2

mov al, '1'
mov [gs:cx], ax
```

### 计算器快速算法

```python
python3
x, y = 12, 12
offset = 2 * (80 * x + y)
print(f"Offset: {offset} (0x{offset:x})")
```

### 完整的 Assignment 1.2 代码框架

```asm
org 0x7c00
[bits 16]

xor ax, ax
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

mov sp, 0x7c00
mov ax, 0xb800
mov gs, ax

; 第一部分：输出 "Hello World"（蓝色，位置 0,0）
mov ah, 0x01
mov al, 'H'
mov [gs:0], ax
; ... 其他字符 ...

; 第二部分：输出学号（不同颜色，位置 12,12）
; 计算偏移：2 * (80*12 + 12) = 1944
mov cx, 1944
mov ah, 0x04           ; 改成你选的颜色
mov al, '你'           ; 改成你的学号第一位
mov [gs:cx], ax
add cx, 2

; ... 继续输出其他位数 ...

jmp $
times 510 - ($ - $$) db 0
db 0x55, 0xaa
```

---

## 第 7 部分：常见错误

### 错误 1：段寄存器直接赋值

```asm
❌ mov gs, 0xB800      ; 错误：不能直接赋值
✅ mov ax, 0xB800      ; 先放到 ax
   mov gs, ax         ; 再赋给 gs
```

### 错误 2：忘记启动标志

```asm
❌ times 510 - ($ - $$) db 0   ; 缺少启动标志！
✅ times 510 - ($ - $$) db 0
   db 0x55, 0xaa              ; 必须有这行
```

### 错误 3：位置计算错误

```
位置 (12, 12) 不是直接 [gs:12]
要用公式：2 × (80 × row + col)
= 2 × (80 × 12 + 12)
= 2 × 972
= 1944
```

### 错误 4：颜色值错误

```asm
❌ mov ah, 0xFF        ; 可能看不见
✅ mov ah, 0x04        ; 红色，看得见
```

---

## 第 8 部分：调试技巧

### 用 GDB 调试

```bash
# Terminal 1：启动 QEMU 并等待调试器
qemu-system-i386 -hda hd.img -s -S -serial null -parallel stdio

# Terminal 2：启动 GDB
gdb

# 在 GDB 中
(gdb) target remote:1234
(gdb) break *0x7c00
(gdb) continue
(gdb) si                 ; 单步
(gdb) info registers     ; 查看寄存器
(gdb) x/10x $gs:$eax    ; 查看内存
```

### 快速验证

```bash
# 查看编译后的二进制文件
hexdump -C hello_mbr.bin | tail -3

# 应该看到最后一行以 "55 aa" 结尾
```

---

## 总结

Assignment 1 考查的核心：

✅ 理解 MBR 的加载过程（0x7C00）  
✅ 理解显存原理（0xB8000）  
✅ 理解段寄存器的使用  
✅ 掌握 16 位汇编基本指令  
✅ 能够计算显存地址  

**完成 1.2 后，你就掌握了最基础的操作系统启动程序！** 🎉
