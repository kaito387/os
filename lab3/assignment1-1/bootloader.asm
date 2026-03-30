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
