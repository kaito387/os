# 操作系统实验3完整指导手册
# 从实模式到保护模式

> 本文档手把手教你完成实验3的所有Assignment

---

## 目录

- [实验背景知识](#实验背景知识)
- [环境准备](#环境准备)
- [Assignment 1.1: 复现Example 1](#assignment-11-复现example-1)
- [Assignment 1.2: CHS模式读取硬盘](#assignment-12-chs模式读取硬盘)
- [Assignment 2: 调试保护模式切换](#assignment-2-调试保护模式切换)
- [Assignment 3: 在保护模式执行自定义程序](#assignment-3-在保护模式执行自定义程序)

---

## 实验背景知识

### 1. 为什么需要突破512字节限制？

计算机启动时，BIOS只会自动加载MBR（主引导记录，512字节）到内存0x7C00处执行。但512字节太小，无法完成复杂的操作系统加载工作。

**解决方案：** MBR负责加载Bootloader（启动加载器），Bootloader再负责：
- 从实模式切换到保护模式
- 加载操作系统内核
- 其他初始化工作

### 2. 内存布局规划

```
0x0000 - 0x7BFF  | 可用内存
0x7C00 - 0x7DFF  | MBR (512字节)
0x7E00 - 0x87FF  | Bootloader (最多5个扇区，2560字节)
0x8800 - 0x???   | GDT (全局描述符表)
0xB8000          | 显存起始地址
```

### 3. LBA28模式读取硬盘

**LBA (Logical Block Addressing)** 使用逻辑扇区号访问硬盘，比CHS模式简单。

**硬盘端口分配：**
- 0x1F0: 数据端口（16位）
- 0x1F2: 扇区数量
- 0x1F3-0x1F6: LBA地址（28位分成4段）
- 0x1F7: 命令/状态端口

**读取步骤：**
1. 设置LBA地址（分段写入0x1F3-0x1F6）
2. 设置扇区数（写入0x1F2）
3. 发送读命令0x20（写入0x1F7）
4. 等待硬盘就绪（轮询0x1F7状态）
5. 从0x1F0读取数据（每个扇区512字节）

---

## 环境准备

### 所需工具

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install nasm qemu-system-x86 gdb make

# 验证安装
nasm -v
qemu-system-i386 --version
gdb --version
```

### 创建工作目录

```bash
cd /home/lht/dev/study/os/lab3
mkdir -p assignment1-1 assignment1-2 assignment2 assignment3
```

---

## Assignment 1.1: 复现Example 1

### 目标
使用LBA28方式从硬盘加载Bootloader到内存并执行。

### 步骤1：创建项目文件

进入工作目录：
```bash
cd assignment1-1
```

### 步骤2：创建 `mbr.asm`

MBR的职责是从硬盘加载Bootloader到内存0x7E00，然后跳转执行。

```nasm
org 0x7c00
[bits 16]
xor ax, ax ; eax = 0
; 初始化段寄存器, 段地址全部设为0
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

; 初始化栈指针
mov sp, 0x7c00
mov ax, 1                ; 逻辑扇区号第0~15位（从扇区1开始）
mov cx, 0                ; 逻辑扇区号第16~31位
mov bx, 0x7e00           ; bootloader的加载地址
load_bootloader:
    call asm_read_hard_disk  ; 读取硬盘
    inc ax                   ; 下一个扇区
    cmp ax, 5                ; 读取5个扇区（扇区1-5）
    jle load_bootloader
jmp 0x0000:0x7e00        ; 远跳转到bootloader

jmp $ ; 死循环（不应该执行到这里）

; ========== LBA28读取硬盘函数 ==========
; 参数：
;   ax = 逻辑扇区号低16位
;   cx = 逻辑扇区号高16位
;   ds:bx = 目标内存地址
; 返回：
;   bx = bx + 512
asm_read_hard_disk:                           
    mov dx, 0x1f3
    out dx, al    ; LBA地址 bit 0-7

    inc dx        ; 0x1f4
    mov al, ah
    out dx, al    ; LBA地址 bit 8-15

    mov ax, cx
    inc dx        ; 0x1f5
    out dx, al    ; LBA地址 bit 16-23

    inc dx        ; 0x1f6
    mov al, ah
    and al, 0x0f
    or al, 0xe0   ; 0xE0 = 1110 0000B (LBA模式 + 主硬盘 + bit 24-27)
    out dx, al

    mov dx, 0x1f2
    mov al, 1
    out dx, al   ; 读取1个扇区

    mov dx, 0x1f7    
    mov al, 0x20     ; 0x20 = 读命令
    out dx,al

    ; 等待硬盘就绪
  .waits:
    in al, dx        ; 读取状态 (dx = 0x1f7)
    and al,0x88      ; 检查 bit7(BUSY) 和 bit3(DRQ)
    cmp al,0x08      ; 等待 BUSY=0, DRQ=1
    jnz .waits                         
    
    ; 读取512字节到地址ds:bx
    mov cx, 256   ; 每次读取1个字（2字节），读256次          
    mov dx, 0x1f0 ; 数据端口
  .readw:
    in ax, dx
    mov [bx], ax
    add bx, 2
    loop .readw
      
    ret

; 填充到510字节，最后两字节是启动签名
times 510 - ($ - $$) db 0
db 0x55, 0xaa
```

**代码解释：**
- `org 0x7c00`：告诉编译器代码将被加载到0x7C00
- `[bits 16]`：16位实模式代码
- 从扇区1开始读取5个扇区（扇区1-5），加载到0x7E00
- `jmp 0x0000:0x7e00`：远跳转，会同时更新CS和IP寄存器

### 步骤3：创建 `bootloader.asm`

Bootloader在屏幕上显示 "run bootloader"。

```nasm
org 0x7e00
[bits 16]
mov ax, 0xb800    ; 显存段地址
mov gs, ax
mov ah, 0x03      ; 青色 (前景色=青, 背景色=黑)
mov ecx, bootloader_tag_end - bootloader_tag  ; 字符串长度
xor ebx, ebx      ; 显存偏移=0 (屏幕左上角)
mov esi, bootloader_tag

output_bootloader_tag:
    mov al, [esi]           ; 读取字符
    mov word[gs:ebx], ax    ; 写入显存 (AL=字符, AH=属性)
    inc esi
    add ebx, 2
    loop output_bootloader_tag

jmp $ ; 死循环

bootloader_tag db 'run bootloader'
bootloader_tag_end:
```

**代码解释：**
- 显存地址：0xB8000，段地址0xB800，偏移0
- 每个字符占2字节：低字节=ASCII码，高字节=属性（颜色）
- 属性0x03：黑底青字

### 步骤4：创建 `makefile`

```makefile
.PHONY: build run clean

build:
	nasm -f bin mbr.asm -o mbr.bin
	nasm -f bin bootloader.asm -o bootloader.bin
	dd if=/dev/zero of=hd.img bs=512 count=100
	dd if=mbr.bin of=hd.img bs=512 count=1 seek=0 conv=notrunc
	dd if=bootloader.bin of=hd.img bs=512 count=5 seek=1 conv=notrunc

run:
	qemu-system-i386 -hda hd.img -serial null -parallel stdio

clean:
	rm -f *.bin hd.img
```

**命令解释：**
- `nasm -f bin`：编译成纯二进制格式
- `dd if=/dev/zero of=hd.img bs=512 count=100`：创建50KB的虚拟硬盘
- `seek=0`：写入扇区0（MBR）
- `seek=1`：写入扇区1开始（Bootloader）
- `conv=notrunc`：不截断文件

### 步骤5：编译和运行

```bash
make build
make run
```

### 预期结果

屏幕左上角显示青色文字：**run bootloader**

### 截图示例

```
运行 qemu 后应该看到：
┌─────────────────────────────┐
│run bootloader               │
│                             │
│                             │
└─────────────────────────────┘
```

---

## Assignment 1.2: CHS模式读取硬盘

### 目标
使用BIOS中断（INT 13H）的CHS模式替代LBA28端口读写。

### 理论知识

#### LBA到CHS转换公式

给定参数：
- **LBA**：逻辑扇区号
- **SPT** (Sectors Per Track)：每磁道扇区数 = 63
- **HPC** (Heads Per Cylinder)：每柱面磁头数 = 18
- **驱动器号**：0x80（第一块硬盘）

转换公式：
```
柱面号 (Cylinder) = LBA ÷ (HPC × SPT)
磁头号 (Head)     = (LBA ÷ SPT) mod HPC
扇区号 (Sector)   = (LBA mod SPT) + 1
```

**注意：** CHS模式下，扇区号从1开始（不是0）！

#### INT 13H, AH=02H - 读取扇区

寄存器设置：
```
AH = 02h      ; 读取扇区功能号
AL = 扇区数   ; 要读取的扇区数量
CH = 柱面号低8位
CL = 扇区号(bit 0-5) + 柱面号高2位(bit 6-7)
DH = 磁头号
DL = 驱动器号 (0x80 = 第一块硬盘)
ES:BX = 缓冲区地址
```

返回值：
```
CF = 0: 成功, AH = 0
CF = 1: 失败, AH = 错误码
```

### 步骤1：创建工作目录

```bash
cd /home/lht/dev/study/os/lab3/assignment1-2
```

### 步骤2：创建 `mbr.asm` (CHS版本)

```nasm
org 0x7c00
[bits 16]
xor ax, ax
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

mov sp, 0x7c00

; 使用CHS模式读取扇区1-5到0x7E00
mov ax, 1                ; 起始LBA扇区号
mov cx, 5                ; 读取5个扇区
mov bx, 0x7e00           ; 目标地址

load_bootloader:
    push ax
    push bx
    push cx
    call asm_read_hard_disk_chs
    pop cx
    pop bx
    pop ax
    inc ax               ; 下一个扇区
    add bx, 512          ; 下一个内存位置
    loop load_bootloader

jmp 0x0000:0x7e00

jmp $

; ========== CHS模式读取硬盘函数 ==========
; 参数：
;   ax = LBA逻辑扇区号
;   es:bx = 目标内存地址
; 使用：
;   SPT = 63 (每磁道扇区数)
;   HPC = 18 (每柱面磁头数)
asm_read_hard_disk_chs:
    push ax
    push bx
    push cx
    push dx

    ; 计算 CHS
    ; 柱面 = LBA / (SPT * HPC) = LBA / (63 * 18) = LBA / 1134
    ; 磁头 = (LBA / SPT) mod HPC = (LBA / 63) mod 18
    ; 扇区 = (LBA mod SPT) + 1 = (LBA mod 63) + 1

    push ax              ; 保存LBA
    
    ; 计算扇区号：(LBA mod 63) + 1
    xor dx, dx
    mov cx, 63           ; SPT
    div cx               ; AX = LBA / 63, DX = LBA mod 63
    inc dx               ; 扇区号从1开始
    mov cl, dl           ; CL = 扇区号 (1-63)
    
    ; 现在 AX = LBA / 63
    ; 计算磁头号：(LBA / 63) mod 18
    xor dx, dx
    mov bx, 18           ; HPC
    div bx               ; AX = 柱面号, DX = 磁头号
    mov dh, dl           ; DH = 磁头号
    
    ; 设置柱面号
    mov ch, al           ; CH = 柱面号低8位
    shl ah, 6
    or cl, ah            ; CL高2位 = 柱面号高2位

    pop ax               ; 恢复栈上保存的值到正确位置
    pop bx               ; ES:BX = 缓冲区地址
    
    ; 调用BIOS中断
    mov dl, 0x80         ; 驱动器号：第一块硬盘
    mov ax, 0x0201       ; AH=02h(读), AL=01h(1个扇区)
    int 0x13             ; BIOS磁盘中断
    
    jc .error            ; CF=1 表示错误
    
    pop dx
    pop cx
    pop ax
    ret

.error:
    ; 错误处理：在屏幕显示'E'
    mov ax, 0xb800
    mov gs, ax
    mov byte [gs:0], 'E'
    mov byte [gs:1], 0x0C  ; 红色
    jmp $

times 510 - ($ - $$) db 0
db 0x55, 0xaa
```

**重要说明：**
1. **CL寄存器的位分配**：
   - bit 0-5：扇区号（1-63）
   - bit 6-7：柱面号的高2位
   
2. **柱面号存储**：
   - CH：柱面号低8位
   - CL bit 6-7：柱面号高2位
   - 总共10位，支持1024个柱面

3. **转换公式验证**：
   - LBA=1: C=0, H=0, S=2
   - LBA=63: C=1, H=0, S=1
   - LBA=1134: C=18, H=0, S=1

### 步骤3：创建 `bootloader.asm`

使用与1.1相同的bootloader.asm。

### 步骤4：创建 `makefile`

```makefile
.PHONY: build run clean

build:
	nasm -f bin mbr.asm -o mbr.bin
	nasm -f bin bootloader.asm -o bootloader.bin
	dd if=/dev/zero of=hd.img bs=512 count=100
	dd if=mbr.bin of=hd.img bs=512 count=1 seek=0 conv=notrunc
	dd if=bootloader.bin of=hd.img bs=512 count=5 seek=1 conv=notrunc

run:
	qemu-system-i386 -hda hd.img -serial null -parallel stdio

clean:
	rm -f *.bin hd.img
```

### 步骤5：编译和运行

```bash
make build
make run
```

### 预期结果

屏幕左上角显示：**run bootloader**（与1.1相同）

### LBA转CHS公式总结

| 参数 | 公式 | 说明 |
|------|------|------|
| 柱面 (Cylinder) | `LBA ÷ (SPT × HPC)` | 整除 |
| 磁头 (Head) | `(LBA ÷ SPT) mod HPC` | 取余 |
| 扇区 (Sector) | `(LBA mod SPT) + 1` | 注意+1 |

其中：
- SPT = 63 (每磁道扇区数)
- HPC = 18 (每柱面磁头数)

---

## Assignment 2: 调试保护模式切换

### 目标
使用GDB在进入保护模式的4个关键步骤设置断点，分析寄存器变化。

### 进入保护模式的4个步骤

1. **准备GDT（全局描述符表）**
2. **打开A20地址线**
3. **设置CR0寄存器的PE位**
4. **执行长跳转刷新流水线**

### 步骤1：准备Example 2代码

```bash
cd /home/lht/dev/study/os/lab3/assignment2
# 复制example-2的文件
cp ../src/example-2/* ./
```

### 步骤2：查看关键代码位置

查看 `bootloader.asm`，找到4个关键步骤的位置：

```nasm
; 步骤1: 设置GDT（行18-35）
mov dword [GDT_START_ADDRESS+0x08],0x0000ffff
...

; 步骤2: 打开A20（行41-43）
in al,0x92
or al,0000_0010B
out 0x92,al

; 步骤3: 设置CR0的PE位（行45-48）
cli
mov eax,cr0
or eax,1
mov cr0,eax

; 步骤4: 长跳转（行51）
jmp dword CODE_SELECTOR:protect_mode_begin
```

### 步骤3：创建 `gdbinit` 调试脚本

```gdb
# 连接到QEMU
target remote localhost:1234

# 设置断点1：准备GDT（在设置第一个描述符之前）
b *0x7e10

# 设置断点2：打开A20（在in al,0x92之前）
b *0x7e29

# 设置断点3：设置CR0（在mov cr0,eax之前）
b *0x7e30

# 设置断点4：长跳转（在jmp指令处）
b *0x7e33

# 显示断点
info breakpoints

# 继续执行
continue
```

**注意：** 实际地址可能需要调整，可以先反汇编确定：
```bash
ndisasm -b 16 bootloader.bin | less
```

### 步骤4：修改 `makefile` 添加调试支持

```makefile
.PHONY: build run debug gdb clean

build:
	nasm -f bin mbr.asm -o mbr.bin
	nasm -f bin bootloader.asm -o bootloader.bin
	dd if=/dev/zero of=hd.img bs=512 count=100
	dd if=mbr.bin of=hd.img bs=512 count=1 seek=0 conv=notrunc
	dd if=bootloader.bin of=hd.img bs=512 count=5 seek=1 conv=notrunc

run:
	qemu-system-i386 -hda hd.img -serial null -parallel stdio

# 调试模式：等待GDB连接
debug:
	qemu-system-i386 -hda hd.img -S -s -serial null -parallel stdio

# 启动GDB
gdb:
	gdb -x gdbinit

clean:
	rm -f *.bin hd.img
```

参数说明：
- `-S`：启动时暂停CPU
- `-s`：在localhost:1234开启GDB服务器

### 步骤5：调试流程

#### 终端1：启动QEMU（调试模式）
```bash
make build
make debug
```

#### 终端2：启动GDB
```bash
make gdb
```

### 步骤6：在GDB中分析每个步骤

#### 断点1：准备GDT

```gdb
(gdb) continue
Breakpoint 1, 0x7e10

# 查看GDT内存（在设置前）
(gdb) x/10xw 0x8800
0x8800: 0x00000000 0x00000000 ...

# 单步执行几条指令，设置GDT
(gdb) si
(gdb) si
...

# 查看GDT内存（在设置后）
(gdb) x/10xw 0x8800
0x8800: 0x00000000 0x00000000  # 空描述符
0x8808: 0x0000ffff 0x00cf9200  # 数据段
0x8810: 0x00000000 0x00409600  # 栈段
0x8818: 0x80007fff 0x0040920b  # 显存段
0x8820: 0x0000ffff 0x00cf9800  # 代码段

# 查看GDTR寄存器
(gdb) info registers gdtr
gdtr  {base=0x8800, limit=39}
```

**分析要点：**
- GDT第一项必须是空描述符（全0）
- 每个描述符8字节
- GDTR保存GDT的基址和界限

#### 断点2：打开A20

```gdb
(gdb) continue
Breakpoint 2, 0x7e29

# 查看当前A20状态
(gdb) info registers eflags

# 单步执行打开A20
(gdb) si    # in al, 0x92
(gdb) si    # or al, 0x02
(gdb) si    # out 0x92, al

# A20已打开，现在可以访问超过1MB的内存
```

**分析要点：**
- A20地址线控制第21位地址线
- 关闭时：地址回绕（1MB边界）
- 打开后：可访问4GB地址空间

#### 断点3：设置CR0的PE位

```gdb
(gdb) continue
Breakpoint 3, 0x7e30

# 查看CR0寄存器（设置前）
(gdb) info registers cr0
cr0  0x60000010

# 单步执行
(gdb) si    # mov eax, cr0
(gdb) info registers eax
eax  0x60000010

(gdb) si    # or eax, 1
(gdb) info registers eax
eax  0x60000011    # PE位(bit 0)已设置

(gdb) si    # mov cr0, eax
(gdb) info registers cr0
cr0  0x60000011    # PE=1，保护模式已启用
```

**分析要点：**
- CR0 bit 0 (PE) = 1：保护模式
- CR0 bit 0 (PE) = 0：实模式
- 其他位控制分页、缓存等功能

#### 断点4：长跳转

```gdb
(gdb) continue
Breakpoint 4, 0x7e33

# 查看当前CS:IP
(gdb) info registers cs eip
cs   0x0      # 16位段地址
eip  0x7e33

# 执行长跳转
(gdb) si    # jmp 0x20:protect_mode_begin

# 查看跳转后的CS:EIP
(gdb) info registers cs eip
cs   0x20     # 代码段选择子
eip  0x7e37   # 32位偏移地址

# 查看当前代码（已经是32位代码）
(gdb) x/10i $eip
```

**分析要点：**
- 长跳转同时修改CS和EIP
- CS从段地址变成段选择子
- CPU切换到32位保护模式
- 清空指令预取队列

### 步骤7：继续执行查看结果

```gdb
(gdb) continue
```

应该看到屏幕显示：
- 第一行：`run bootloader` (16位代码输出)
- 第二行：`enter protect mode` (32位代码输出)

### 实验报告要点

在报告中需要包含：

1. **每个断点的截图**，显示：
   - 断点位置
   - 相关寄存器的值
   - 关键内存内容

2. **每个步骤的分析**：
   - 步骤1：GDT的结构和各段描述符的含义
   - 步骤2：A20的作用和为什么需要打开
   - 步骤3：CR0寄存器各位的含义，PE位的作用
   - 步骤4：为什么需要长跳转，它做了什么

3. **寄存器变化对比表**：

| 步骤 | CR0 | CS | EIP | GDTR | 模式 |
|------|-----|----|----|------|------|
| 初始 | 0x60000010 | 0x0000 | 0x7e00 | 未设置 | 实模式 |
| 设置GDT后 | 0x60000010 | 0x0000 | 0x7e2x | {0x8800, 39} | 实模式 |
| 打开A20后 | 0x60000010 | 0x0000 | 0x7e2x | {0x8800, 39} | 实模式 |
| 设置PE后 | 0x60000011 | 0x0000 | 0x7e3x | {0x8800, 39} | 保护模式 |
| 长跳转后 | 0x60000011 | 0x0020 | 0x7e37 | {0x8800, 39} | 保护模式 |

---

## Assignment 3: 在保护模式执行自定义程序

### 目标
改造Lab2-Assignment4，在保护模式下执行自定义汇编程序。

### 设计思路

1. 复用Example 2的框架（进入保护模式）
2. 在32位保护模式代码段添加自定义功能
3. 实现Lab2中的功能（如屏幕输出、图形绘制等）

### 示例：在保护模式下绘制彩色方块

#### 步骤1：创建工作目录

```bash
cd /home/lht/dev/study/os/lab3/assignment3
```

#### 步骤2：复制Example 2文件

```bash
cp ../src/example-2/boot.inc ./
cp ../src/example-2/mbr.asm ./
```

#### 步骤3：创建 `bootloader.asm`（增强版）

```nasm
%include "boot.inc"
org 0x7e00
[bits 16]

; ========== 16位实模式部分 ==========
mov ax, 0xb800
mov gs, ax
mov ah, 0x03
mov ecx, bootloader_tag_end - bootloader_tag
xor ebx, ebx
mov esi, bootloader_tag
output_bootloader_tag:
    mov al, [esi]
    mov word[gs:ebx], ax
    inc esi
    add ebx, 2
    loop output_bootloader_tag

; 设置GDT
mov dword [GDT_START_ADDRESS+0x00],0x00000000
mov dword [GDT_START_ADDRESS+0x04],0x00000000  

mov dword [GDT_START_ADDRESS+0x08],0x0000ffff
mov dword [GDT_START_ADDRESS+0x0c],0x00cf9200

mov dword [GDT_START_ADDRESS+0x10],0x00000000
mov dword [GDT_START_ADDRESS+0x14],0x00409600

mov dword [GDT_START_ADDRESS+0x18],0x80007fff
mov dword [GDT_START_ADDRESS+0x1c],0x0040920b

mov dword [GDT_START_ADDRESS+0x20],0x0000ffff
mov dword [GDT_START_ADDRESS+0x24],0x00cf9800

mov word [pgdt], 39
lgdt [pgdt]
      
in al,0x92
or al,0000_0010B
out 0x92,al

cli
mov eax,cr0
or eax,1
mov cr0,eax
      
jmp dword CODE_SELECTOR:protect_mode_begin

; ========== 32位保护模式部分 ==========
[bits 32]           
protect_mode_begin:                              
    mov eax, DATA_SELECTOR
    mov ds, eax
    mov es, eax
    mov eax, STACK_SELECTOR
    mov ss, eax
    mov esp, 0x7c00       ; 设置栈指针
    mov eax, VIDEO_SELECTOR
    mov gs, eax

    ; 清屏
    call clear_screen

    ; 输出提示信息
    mov esi, protect_mode_tag
    mov edi, 80 * 2       ; 第二行
    mov ah, 0x0A          ; 绿色
    call print_string

    ; 绘制彩色方块
    call draw_color_blocks

    jmp $

; ========== 32位子程序 ==========

; 清屏函数
clear_screen:
    push eax
    push ecx
    push edi
    mov ecx, 80 * 25      ; 80列 x 25行
    mov edi, 0
    mov ax, 0x0720        ; 空格，白色
.loop:
    mov word [gs:edi], ax
    add edi, 2
    loop .loop
    pop edi
    pop ecx
    pop eax
    ret

; 打印字符串函数
; 参数: esi = 字符串地址, edi = 显存偏移, ah = 颜色
print_string:
    push eax
    push esi
    push edi
.loop:
    mov al, [esi]
    cmp al, 0
    je .done
    mov word [gs:edi], ax
    add edi, 2
    inc esi
    jmp .loop
.done:
    pop edi
    pop esi
    pop eax
    ret

; 绘制彩色方块
draw_color_blocks:
    push eax
    push ebx
    push ecx
    push edx
    
    mov ebx, 4            ; 起始行
    mov ecx, 8            ; 8种颜色
    mov dl, 0             ; 颜色从0开始
    
.color_loop:
    push ecx
    push ebx
    
    ; 绘制5x10的方块
    mov ecx, 5            ; 5行
.row_loop:
    push ecx
    push ebx
    
    ; 计算显存位置: (row * 80 + col) * 2
    mov eax, ebx
    imul eax, 80
    add eax, 10           ; 从第10列开始
    add eax, edx          ; 偏移根据颜色
    add eax, edx          ; x2
    add eax, edx          ; x3 (颜色间距)
    add eax, edx          ; x4
    add eax, edx          ; x5
    shl eax, 1            ; x2 (每个字符2字节)
    mov edi, eax
    
    ; 绘制一行（10个字符）
    mov ecx, 10
.col_loop:
    mov al, ' '           ; 空格
    mov ah, dl            ; 颜色
    or ah, 0xF0           ; 背景色
    mov word [gs:edi], ax
    add edi, 2
    loop .col_loop
    
    pop ebx
    pop ecx
    inc ebx               ; 下一行
    loop .row_loop
    
    pop ebx
    pop ecx
    inc dl                ; 下一种颜色
    loop .color_loop
    
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

; ========== 数据段 ==========
pgdt dw 0
     dd GDT_START_ADDRESS

bootloader_tag db 'Loading...', 0
bootloader_tag_end:

protect_mode_tag db 'Welcome to 32-bit Protected Mode!', 0
```

#### 步骤4：创建 `makefile`

```makefile
.PHONY: build run clean

build:
	nasm -f bin mbr.asm -o mbr.bin
	nasm -f bin bootloader.asm -o bootloader.bin
	dd if=/dev/zero of=hd.img bs=512 count=100
	dd if=mbr.bin of=hd.img bs=512 count=1 seek=0 conv=notrunc
	dd if=bootloader.bin of=hd.img bs=512 count=5 seek=1 conv=notrunc

run:
	qemu-system-i386 -hda hd.img

clean:
	rm -f *.bin hd.img
```

#### 步骤5：编译运行

```bash
make build
make run
```

### 预期结果

屏幕显示：
1. 第一行：`Loading...` (16位代码)
2. 第二行：`Welcome to 32-bit Protected Mode!` (32位代码)
3. 下方：8个不同颜色的方块

### 扩展功能建议

根据Lab2-Assignment4的内容，可以实现：

1. **字符串输出**：打印学号、姓名
2. **图形绘制**：
   - 矩形框
   - 填充方块
   - 简单的图案
3. **颜色渐变**：遍历16种颜色
4. **动画效果**（可选）：
   - 使用循环延时
   - 改变方块位置或颜色

### 32位保护模式的优势

1. **4GB地址空间**：不再受1MB限制
2. **32位寄存器**：EAX, EBX等可处理更大数值
3. **内存保护**：段权限控制
4. **更强的指令集**：支持更多指令

---

## 附录

### A. 常用GDB命令

| 命令 | 说明 |
|------|------|
| `target remote localhost:1234` | 连接QEMU |
| `b *0x7c00` | 在地址0x7c00设置断点 |
| `continue` (或 `c`) | 继续执行 |
| `si` | 单步执行（Step Instruction） |
| `ni` | 单步跳过函数 |
| `info registers` | 查看所有寄存器 |
| `info registers eax` | 查看特定寄存器 |
| `x/10xb 0x7c00` | 查看内存（十六进制字节） |
| `x/10i $eip` | 反汇编当前指令 |
| `print $eax` | 打印表达式 |
| `layout asm` | 显示汇编窗口 |
| `quit` | 退出GDB |

### B. 段描述符格式（32位保护模式）

```
63                                                              32
┌─────┬────┬────┬────┬────┬─────┬────┬────┬────┬────┬────────────────┐
│Base │ G  │D/B │ L  │AVL │Limit│ P  │DPL │  S │Type│   Base 24-31   │
│31-24│    │    │    │    │19-16│    │    │    │    │                │
└─────┴────┴────┴────┴────┴─────┴────┴────┴────┴────┴────────────────┘

31                                                               0
┌──────────────────────────────────────────────────────────────┐
│           Base Address 15-0            │    Limit 15-0       │
└──────────────────────────────────────────────────────────────┘
```

**字段说明：**
- **Base**: 段基址（32位）
- **Limit**: 段界限（20位）
- **G**: 粒度（0=字节，1=4KB）
- **D/B**: 默认操作数大小（0=16位，1=32位）
- **P**: 存在位（1=在内存中）
- **DPL**: 特权级（0-3）
- **S**: 描述符类型（1=代码/数据段，0=系统段）
- **Type**: 段类型（可读、可写、可执行等）

### C. 实模式 vs 保护模式对比

| 特性 | 实模式 | 保护模式 |
|------|--------|----------|
| 地址空间 | 1MB (20位) | 4GB (32位) |
| 寄存器 | 16位 | 32位 |
| 段寄存器 | 段基址 | 段选择子 |
| 地址计算 | Seg*16+Offset | 查GDT获取基址+偏移 |
| 内存保护 | 无 | 有（特权级） |
| 中断 | IVT (0x0000) | IDT（可配置） |

### D. 常见错误排查

#### 1. QEMU启动后黑屏
- 检查MBR是否写入扇区0
- 检查启动签名：0x55, 0xAA
- 使用 `xxd hd.img | head` 查看镜像内容

#### 2. 三重故障（Triple Fault）
- GDT设置错误
- 段选择子不正确
- 栈指针未初始化
- 使用 `qemu-system-i386 -d int` 查看中断日志

#### 3. 进入保护模式后无输出
- 检查显存段描述符（基址0xB8000）
- 确认段选择子正确加载到GS
- 检查是否使用32位指令（[bits 32]）

#### 4. CHS读取失败
- 验证转换公式
- 检查SPT和HPC参数
- 使用CF标志检测错误：`jc error`

### E. 参考资料

1. **《x86汇编语言：从实模式到保护模式》** - 李忠
2. **Intel® 64 and IA-32 Architectures Software Developer's Manual**
3. **OSDev Wiki**: https://wiki.osdev.org
4. **QEMU文档**: https://www.qemu.org/docs/master/
5. **GDB调试指南**: https://sourceware.org/gdb/documentation/

### F. 完整项目结构

```
lab3/
├── assignment1-1/
│   ├── mbr.asm
│   ├── bootloader.asm
│   ├── makefile
│   └── hd.img (生成)
├── assignment1-2/
│   ├── mbr.asm (CHS版本)
│   ├── bootloader.asm
│   ├── makefile
│   └── hd.img (生成)
├── assignment2/
│   ├── boot.inc
│   ├── mbr.asm
│   ├── bootloader.asm
│   ├── gdbinit
│   ├── makefile
│   └── hd.img (生成)
├── assignment3/
│   ├── boot.inc
│   ├── mbr.asm
│   ├── bootloader.asm (增强版)
│   ├── makefile
│   └── hd.img (生成)
└── 实验报告.md
```

---

## 实验报告模板

使用之前创建的 `report-template.md`，填写每个Assignment的：

1. **实验要求**：复述assignment目标
2. **实验过程**：
   - 创建的文件
   - 关键代码片段
   - 编译和运行步骤
3. **关键代码**：
   - LBA/CHS读取函数
   - GDT设置
   - 保护模式切换
   - 自定义32位程序
4. **实验结果**：
   - QEMU运行截图
   - GDB调试截图（Assignment 2）
   - 寄存器状态截图
5. **总结**：
   - 遇到的问题和解决方案
   - 对实模式和保护模式的理解
   - 改进建议

---

## 提交检查清单

- [ ] Assignment 1.1 代码完整且能运行
- [ ] Assignment 1.2 CHS代码正确，转换公式正确
- [ ] Assignment 2 GDB调试截图齐全（4个断点）
- [ ] Assignment 3 自定义32位程序有创新点
- [ ] 实验报告格式规范，截图清晰
- [ ] 压缩包命名：`lab3-姓名-学号.zip`
- [ ] 压缩包包含所有代码和实验报告PDF

---

**祝你实验顺利！有问题随时查阅本手册。**

**记住：**
- 每一步都要理解原理，不要盲目复制
- 多用GDB调试，观察寄存器和内存变化
- 遇到问题先自己排查，再寻求帮助
- 保持代码整洁和注释清晰
