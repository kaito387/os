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

mov di, 1944

mov ah, 0x02 ; green      

mov al, '2'
mov [gs:di], ax
add di, 2

mov al, '4'
mov [gs:di], ax
add di, 2

mov al, '3'
mov [gs:di], ax
add di, 2

mov al, '4'
mov [gs:di], ax
add di, 2

mov al, '4'
mov [gs:di], ax
add di, 2

mov al, '0'
mov [gs:di], ax
add di, 2

mov al, '6'
mov [gs:di], ax
add di, 2

mov al, '4'
mov [gs:di], ax
add di, 2

jmp $
times 510 - ($ - $$) db 0
db 0x55, 0xaa
