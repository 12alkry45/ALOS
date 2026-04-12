[org 0x7C00]
    mov bp, 0x9000
    mov sp, bp

    mov bx, MSG_REAL_MODE
    call print
    call print_nl

    call switch_to_pm
    jmp $

%include "boot_print.asm"
%include "gdt.asm"
%include "print_32bit_mode.asm"
%include "switch_32bit.asm"

[bits 32]
BEGIN_PM:
    mov ebx, MSG_PROTECTED_MODE
    call print_string_pm
    jmp $

MSG_REAL_MODE db "Started in 16-bit real mode", 0
MSG_PROTECTED_MODE db "Loaded 32-bit protected mode", 0

times 510-($-$$) db 0
dw 0xAA55
