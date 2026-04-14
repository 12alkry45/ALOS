; load 'dh' sectors from drive 'dl' into ES:BX; dl gets from BIOS; es:bx
disk_load:
    pusha
    push dx

    mov ah, 0x02 ; ah <- int 0x13 read function
    mov al, dh   ; number of sectors
    mov cl, 0x02 ; sector; 0x01 is boot sector
    mov ch, 0x00 ; cylinder
    mov dh, 0x00 ; head number
    
    int 0x13
    jc disk_error

    pop dx
    cmp al, dh   ; how many sectors read
    jne sectors_error
    popa
    ret

disk_error:
    mov bx, DISK_ERROR  
    call print
    call print_nl
    mov dh, ah    ; error code, dl disk drive dropped error
    call print_hex
    jmp disk_loop

sectors_error:
    mov bx, SECTORS_ERROR 
    call print

disk_loop:
    jmp $

DISK_ERROR: db "Disk read error", 0
SECTORS_ERROR: db "Incorrect number of sectors read", 0
