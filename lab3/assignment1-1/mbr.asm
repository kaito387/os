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
jmp 0x0000:0x7e00        ; 跳转到bootloader

jmp $ ; 死循环

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

times 510 - ($ - $$) db 0
db 0x55, 0xaa
