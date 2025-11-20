#include "gdt.h"

struct gdt_entry gdt_entries[6];
struct gdt_ptr gdt_pointer;

struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

struct tss_entry tss;
uint8_t kernel_stack[8192] __attribute__((aligned(16)));

static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;
    
    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access = access;
}

extern "C" void tss_flush();

void gdt_init() {
    gdt_pointer.limit = (sizeof(struct gdt_entry) * 6) - 1;
    gdt_pointer.base = (uint32_t)&gdt_entries;
    
    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment
    
    uint32_t tss_base = (uint32_t)&tss;
    uint32_t tss_limit = sizeof(tss) - 1;
    gdt_set_gate(5, tss_base, tss_limit, 0x89, 0x00); // TSS (32-bit available)
    
    for (uint32_t i = 0; i < sizeof(tss); i++) {
        ((uint8_t*)&tss)[i] = 0;
    }
    
    tss.ss0 = 0x10;
    tss.esp0 = (uint32_t)kernel_stack + sizeof(kernel_stack);
    tss.iomap_base = sizeof(tss);
    
    gdt_flush((uint32_t)&gdt_pointer);
    tss_flush();
}
