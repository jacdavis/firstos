.global process_enter_usermode

process_enter_usermode:
    cli
    
    mov 4(%esp), %ecx
    mov 8(%esp), %ebx
    
    push $0x23
    push %ebx
    pushf
    or $0x200, (%esp)
    push $0x1B
    push %ecx
    
    iret
