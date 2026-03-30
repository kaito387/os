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
