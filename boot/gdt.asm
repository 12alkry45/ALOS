gdt_start: 
    dd 0x0 ; 4 byte
    dd 0x0 ; 4 byte
; GDT for code segment; base = 0x00000000, length = 0xFFFFF
gdt_code:
    dw 0xFFFF    ; length, bits 0-15
    dw 0x0       ; segment base bits 0-15
    db 0x0       ; segment base bints 16-23
    db 10011010b ; flags 8bit
    db 11001111b ; flags 4bit + segment length, bits 16-19
    db 0x0       ; segment base, bits 24-31
; GDT for data segment; base = 0x00000000, length = 0xFFFFF
gdt_data:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0
gdt_end:

; GDT descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; size 16 bits
    dd gdt_start               ; address 32 bits

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
