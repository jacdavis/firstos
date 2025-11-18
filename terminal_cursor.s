.global terminal_update_cursor_asm
.type terminal_update_cursor_asm, @function

terminal_update_cursor_asm:
    mov 4(%esp), %ax
    
    push %eax
    mov $0x0F, %al
    mov $0x3D4, %dx
    outb %al, %dx
    pop %eax
    
    mov $0x3D5, %dx
    outb %al, %dx
    
    shr $8, %ax
    
    push %eax
    mov $0x0E, %al
    mov $0x3D4, %dx
    outb %al, %dx
    pop %eax
    
    mov $0x3D5, %dx
    outb %al, %dx
    
    ret
