org 0x7C00
[bits 16]

start:
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00

    mov ax, 0xB800
    mov es, ax

    mov byte [ds:row], 2
    mov byte [ds:col], 0
    mov byte [ds:dr], 1
    mov byte [ds:dc], 1
    mov byte [ds:attr], 0x1F
    mov byte [ds:chr], 0x20

main_loop:
    xor ax, ax
    mov al, [ds:row]        ; al = row
    mul word [ds:const80]   ; ax = row * 80

    xor bx, bx
    mov bl, [ds:col]        ; bl = col
    add ax, bx              ; ax = row * 80 + col
    shl ax, 1               ; ax = 2 (row * 80 + col), 字符所在现存地址
    mov di, ax

    mov al, [ds:chr]
    mov ah, [ds:attr]
    mov [es:di], ax         ; 写入显存

    mov cx, 0x000f
delay:
    nop
    mov dx, 0xffff
.inner:
    nop
    dec dx
    jnz .inner              ; 设置延迟, 放置字符打印过快

    dec cx
    jnz delay

    add byte [ds:chr], 1
    mov al, [ds:chr]
    cmp al, 0x7f
    jne .ch_not_last
    mov byte [ds:chr], 0x20
.ch_not_last:               ; 修改字符, 在 ASCII 0x20~0x7e 循环

    inc byte [ds:attr]      ; 修改字符属性, 自然溢出循环

    mov al, [ds:row]
    add al, [ds:dr]
    cmp al, 25              ; 是否到末尾
    jne .r_not_bottom
    mov al, 23
    neg byte [ds:dr]
.r_not_bottom:
    cmp al, 0FFh            ; 是否到 -1 (开头)
    jne .r_store
    xor al, al
    neg byte [ds:dr]
.r_store:                   ; 存储 row
    mov [ds:row], al

    mov al, [ds:col]
    add al, [ds:dc]
    cmp al, 80              ; hit right?
    jne .c_not_right
    mov al, 78
    neg byte [ds:dc]
.c_not_right:
    cmp al, 0FFh            ; hit left?
    jne .c_store
    xor al, al
    neg byte [ds:dc]
.c_store:                   ; store column
    mov [ds:col], al

    jmp main_loop

const80 dw 80
row    db 0
col    db 0
dr     db 0
dc     db 0
attr   db 0
chr    db 0

times 510-($-$$) db 0
dw 0xAA55
