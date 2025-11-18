.global inb
.type inb, @function
inb:
    mov 4(%esp), %dx
    xor %eax, %eax
    inb %dx, %al
    ret

.global outb
.type outb, @function
outb:
    mov 4(%esp), %dx
    mov 8(%esp), %al
    outb %al, %dx
    ret
