.global paging_load_directory
.global paging_enable

paging_load_directory:
    mov 4(%esp), %eax
    mov %eax, %cr3
    ret

paging_enable:
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0
    ret
