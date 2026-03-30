%include "boot.inc"
org 0x7e00

[bits 16]

; ========== 16位实模式部分 ==========
mov ax, 0xb800
mov gs, ax
mov ah, 0x03 ;青色
mov ecx, bootloader_tag_end - bootloader_tag
xor ebx, ebx
mov esi, bootloader_tag
output_bootloader_tag:
    mov al, [esi]
    mov word[gs:bx], ax
    inc esi
    add ebx,2
    loop output_bootloader_tag

; 设置 GDT
mov dword [GDT_START_ADDRESS+0x00],0x00
mov dword [GDT_START_ADDRESS+0x04],0x00  

mov dword [GDT_START_ADDRESS+0x08],0x0000ffff    ; 基地址为0，段界限为0xFFFFF
mov dword [GDT_START_ADDRESS+0x0c],0x00cf9200    ; 粒度为4KB，存储器段描述符 

mov dword [GDT_START_ADDRESS+0x10],0x00000000    ; 基地址为0x00000000，界限0x0 
mov dword [GDT_START_ADDRESS+0x14],0x00409600    ; 粒度为1个字节

mov dword [GDT_START_ADDRESS+0x18],0x80007fff    ; 基地址为0x000B8000，界限0x07FFF 
mov dword [GDT_START_ADDRESS+0x1c],0x0040920b    ; 粒度为字节

mov dword [GDT_START_ADDRESS+0x20],0x0000ffff    ; 基地址为0，段界限为0xFFFFF
mov dword [GDT_START_ADDRESS+0x24],0x00cf9800    ; 粒度为4kb，代码段描述符 

mov word [pgdt], 39      ;描述符表的界限   
lgdt [pgdt]
      
in al,0x92                         ;南桥芯片内的端口 
or al,0000_0010B
out 0x92,al                        ;打开A20

cli                                ;中断机制尚未工作
mov eax,cr0
or eax,1
mov cr0,eax                        ;设置PE位 
      
jmp dword CODE_SELECTOR:protect_mode_begin  

;16位的描述符选择子：32位偏移
;清流水线并串行化处理器
; ========== 32位保护模式部分 ==========
[bits 32]           
protect_mode_begin:
    ; 初始化段寄存器
    mov eax, DATA_SELECTOR
    mov ds, eax
    mov es, eax
    mov eax, STACK_SELECTOR
    mov ss, eax
    mov esp, 0x7c00
    mov eax, VIDEO_SELECTOR
    mov gs, eax

    ; 清屏
    call clear_screen

    ; 初始化弹射变量
    mov byte [row], 2
    mov byte [col], 0
    mov byte [dr], 1
    mov byte [dc], 1
    mov byte [attr], 0x1F
    mov byte [chr], 0x20

; ========== 主循环：字符弹射 ==========
main_loop:
    ; 计算显存位置: (row * 80 + col) * 2
    xor eax, eax
    mov al, [row]           ; eax = row
    imul eax, 80            ; eax = row * 80
    
    xor ebx, ebx
    mov bl, [col]           ; ebx = col
    add eax, ebx            ; eax = row * 80 + col
    shl eax, 1              ; eax *= 2 (每个字符2字节)
    mov edi, eax            ; edi = 显存偏移

    ; 写入字符到显存
    mov al, [chr]           ; AL = 字符
    mov ah, [attr]          ; AH = 属性
    mov word [gs:edi], ax

    ; 延迟（32位版本）
    mov ecx, 0x001fffff     ; 外层循环
delay:
    nop
    dec ecx
    jnz delay

    ; 更新字符（0x20~0x7e循环）
    inc byte [chr]
    mov al, [chr]
    cmp al, 0x7f
    jne .chr_not_last
    mov byte [chr], 0x20
.chr_not_last:

    ; 更新属性（自然溢出）
    inc byte [attr]

    ; 更新行位置（带边界检测）
    mov al, [row]
    movsx eax, al           ; 符号扩展到32位
    mov bl, [dr]
    movsx ebx, bl
    add eax, ebx            ; row += dr
    
    cmp al, 25              ; 到达底部？
    jne .row_not_bottom
    mov al, 23
    neg byte [dr]
.row_not_bottom:
    cmp al, -1              ; 到达顶部？
    jne .row_store
    mov al, 1
    neg byte [dr]
.row_store:
    mov [row], al

    ; 更新列位置（带边界检测）
    mov al, [col]
    movsx eax, al
    mov bl, [dc]
    movsx ebx, bl
    add eax, ebx            ; col += dc
    
    cmp al, 80              ; 到达右边？
    jne .col_not_right
    mov al, 78
    neg byte [dc]
.col_not_right:
    cmp al, -1              ; 到达左边？
    jne .col_store
    mov al, 1
    neg byte [dc]
.col_store:
    mov [col], al

    jmp main_loop           ; 无限循环

; ========== 辅助函数 ==========

; 清屏函数
clear_screen:
    push eax
    push ecx
    push edi
    
    mov ecx, 80 * 25        ; 2000个字符
    mov edi, 0
    mov ax, 0x0720          ; 空格 + 白色
.loop:
    mov word [gs:edi], ax
    add edi, 2
    loop .loop
    
    pop edi
    pop ecx
    pop eax
    ret

; ========== 数据段 ==========
pgdt dw 0
     dd GDT_START_ADDRESS

bootloader_tag db 'Loading Bounce Program...', 0
bootloader_tag_end:

; 弹射程序变量（32位保护模式下访问）
row    db 0
col    db 0
dr     db 0    ; 行方向 (+1 或 -1)
dc     db 0    ; 列方向 (+1 或 -1)
attr   db 0    ; 字符属性（颜色）
chr    db 0    ; 当前字符
