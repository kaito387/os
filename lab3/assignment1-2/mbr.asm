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
    call asm_read_hard_disk_chs
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

    push cx
    push ax
    push bx
    ; 计算扇区号：(LBA mod 63) + 1
    xor dx, dx
    mov cx, 63           ; SPT
    div cx               ; AX = LBA / 63, DX = LBA mod 63
    inc dx               ; DX = 扇区号
    mov cl, dl           ; CL = 扇区号
    
    ; 现在 AX = LBA / 63
    ; 磁头号 = (LBA / 63) mod 18
    xor dx, dx
    mov bx, 18           ; HPC
    div bx               ; AX = 柱面号, DX = 磁头号
    mov dh, dl           ; DH = 磁头号
    
    ; 柱面号 = LBA / (SPT * HPC) = LBA / (63 * 18) = LBA / 1134
    mov ch, al           ; CH = 柱面号低8位
    shl ah, 6
    or cl, ah            ; CL高2位 = 柱面号高2位

    pop bx

    ; 调用BIOS中断
    mov dl, 0x80         ; 驱动器号：第一块硬盘
    mov ax, 0x0201       ; AH=02h(读), AL=01h(1个扇区)
    int 0x13             ; BIOS磁盘中断

    pop ax
    pop cx
    
    ret

times 510 - ($ - $$) db 0
db 0x55, 0xaa
