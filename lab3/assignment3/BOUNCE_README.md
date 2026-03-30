# 字符弹射程序 - 32位保护模式版本

## 主要改动说明

### 1. 寄存器变化
| 16位实模式 | 32位保护模式 | 说明 |
|-----------|-------------|------|
| `ax, bx, cx, dx` | `eax, ebx, ecx, edx` | 32位寄存器 |
| `mul word [const80]` | `imul eax, 80` | 32位乘法更简单 |
| `shl ax, 1` | `shl eax, 1` | 位移操作 |

### 2. 段寄存器使用
```asm
; 16位：直接设置段地址
mov ax, 0xB800
mov es, ax

; 32位：通过段选择子
mov eax, VIDEO_SELECTOR  ; 0x18, 指向GDT中的显存段描述符
mov gs, eax
```

### 3. 数据访问方式
```asm
; 16位：通过段寄存器
mov al, [ds:row]

; 32位：数据段已设置，直接访问
mov al, [row]
```

### 4. 符号扩展
```asm
; 处理负数时需要符号扩展
movsx eax, al    ; 将AL符号扩展到EAX
movsx ebx, bl    ; 将BL符号扩展到EBX
```

### 5. 延迟循环优化
```asm
; 32位CPU更快，需要更大的循环次数
mov ecx, 0x000fffff  ; 比16位大得多
```

## 编译和运行

```bash
cd /home/lht/dev/study/os/lab3/assignment3

# 确保有boot.inc（从example-2复制）
cp ../src/example-2/boot.inc ./

# 使用官方makefile（支持调试符号）
# 或使用简化版本：
nasm -f bin mbr.asm -o mbr.bin
nasm -f bin bootloader_bounce.asm -o bootloader_bounce.bin
dd if=/dev/zero of=hd.img bs=512 count=100
dd if=mbr.bin of=hd.img bs=512 count=1 seek=0 conv=notrunc
dd if=bootloader_bounce.bin of=hd.img bs=512 count=5 seek=1 conv=notrunc

qemu-system-i386 -hda hd.img
```

## 预期效果

1. **启动**：屏幕显示 "Loading Bounce Program..."
2. **清屏**：进入保护模式后清空屏幕
3. **弹射**：字符在屏幕内弹跳
   - 字符从 `0x20` (空格) 循环到 `0x7e` (~)
   - 颜色属性自动循环（256种组合）
   - 碰到边界自动反弹

## 关键技术点

### 1. 32位寻址优势
```asm
; 16位需要分段计算
mov ax, row
mul word [const80]  ; 需要额外的常量

; 32位直接计算
imul eax, 80         ; 直接立即数乘法
```

### 2. 边界检测改进
```asm
; 使用符号扩展处理负数
movsx eax, al        ; 如果al=-1，eax=0xFFFFFFFF
cmp al, -1           ; 可以直接比较
```

### 3. 显存访问
```asm
; 通过GS段选择子访问显存
; GDT中显存段描述符：基址0xB8000, 界限0x7FFF
mov word [gs:edi], ax  ; 直接写入显存
```

## 与16位版本对比

| 特性 | 16位实模式 | 32位保护模式 |
|------|-----------|------------|
| 地址空间 | 1MB | 4GB |
| 寄存器 | 16位 | 32位 |
| 计算效率 | 需要常量表 | 立即数运算 |
| 段管理 | 手动设置段地址 | GDT自动管理 |
| 安全性 | 无保护 | 段级保护 |
| 中断 | BIOS中断可用 | 需要自建IDT |

## 可选扩展

1. **多字符弹射**：维护多个字符的坐标
2. **轨迹显示**：保留移动轨迹
3. **碰撞检测**：多个字符互相碰撞
4. **键盘控制**：修改弹射速度或方向（需实现键盘中断）
