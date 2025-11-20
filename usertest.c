void syscall_write(const char* msg, int len) {
    asm volatile(
        "mov $1, %%eax\n"
        "mov %0, %%ebx\n"
        "mov %1, %%ecx\n"
        "int $0x80\n"
        :
        : "r"(msg), "r"(len)
        : "eax", "ebx", "ecx"
    );
}

void _start() {
    asm volatile(
        "mov $0x23, %ax\n"
        "mov %ax, %ds\n"
        "mov %ax, %es\n"
        "mov %ax, %fs\n"
        "mov %ax, %gs\n"
    );
    
    syscall_write("Hello from user mode!\n", 22);
    
    while(1) {
        asm volatile("nop");
    }
}
