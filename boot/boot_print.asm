; print sentence from bx
print:
    pusha
start:
    mov al, [bx]
    cmp al, 0
    je done

    mov ah, 0x0E ;tty
    int 0x10

    inc bx
    jmp start
done:
    popa
    ret

print_nl:
    pusha
    mov ah, 0x0E
    mov al, 0x0A ; newline
    int 0x10
    mov al, 0x0D ;caret back
    int 0x10
    popa
    ret
