[org 0x7C00]
    mov bp, 0x0800
    mov sp, bp

    mov bx, 0x9000
    mov dh, 2
    call disk_load

    mov dx, [0x9000]
    call print_hex
    call print_nl

    mov dx, [0x9200]
    call print_hex
    call print_nl

    jmp $

%include "boot_print.asm"
%include "boot_print_hex.asm"
%include "boot_sect_disk.asm"

times 510-($-$$) db 0
dw 0xAA55

times 256 dw 0xDADA ; sector 2
times 256 dw 0xFADE ; sector 3
