#include "process.h"
#include "paging.h"
#include "pmm.h"
#include "elf.h"

struct process* current_process = 0;
static uint32_t next_pid = 1;

extern "C" void process_enter_usermode(uint32_t eip, uint32_t esp);

void process_init() {
    current_process = 0;
}

void* memcpy(void* dest, const void* src, uint32_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (uint32_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

void* memset(void* s, int c, uint32_t n) {
    uint8_t* p = (uint8_t*)s;
    for (uint32_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
    return s;
}

struct process* process_create_from_elf(void* elf_data, uint32_t size) {
    Elf32_Ehdr* elf_header = (Elf32_Ehdr*)elf_data;
    
    if (elf_header->e_ident[0] != 0x7F ||
        elf_header->e_ident[1] != 'E' ||
        elf_header->e_ident[2] != 'L' ||
        elf_header->e_ident[3] != 'F') {
        return 0;
    }
    
    struct process* proc = (struct process*)pmm_alloc_frame();
    proc->pid = next_pid++;
    proc->page_dir = paging_create_address_space();
    proc->state = PROCESS_STATE_READY;
    
    Elf32_Phdr* program_headers = (Elf32_Phdr*)((uint8_t*)elf_data + elf_header->e_phoff);
    
    for (int i = 0; i < elf_header->e_phnum; i++) {
        if (program_headers[i].p_type == PT_LOAD) {
            uint32_t vaddr = program_headers[i].p_vaddr;
            uint32_t memsz = program_headers[i].p_memsz;
            uint32_t filesz = program_headers[i].p_filesz;
            uint32_t offset = program_headers[i].p_offset;
            
            uint32_t start_page = vaddr & 0xFFFFF000;
            uint32_t end_page = ((vaddr + memsz) & 0xFFFFF000) + PAGE_SIZE;
            
            for (uint32_t page = start_page; page < end_page; page += PAGE_SIZE) {
                uint32_t frame = pmm_alloc_frame();
                paging_map_page(proc->page_dir, page, frame, 
                               PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
            }
            
            paging_switch_directory(proc->page_dir);
            
            if (filesz > 0) {
                memcpy((void*)vaddr, (uint8_t*)elf_data + offset, filesz);
            }
            
            if (memsz > filesz) {
                memset((void*)(vaddr + filesz), 0, memsz - filesz);
            }
            
            paging_switch_directory(kernel_directory);
        }
    }
    
    uint32_t stack_top = 0xC0000000;
    for (uint32_t i = 0; i < 4; i++) {
        uint32_t frame = pmm_alloc_frame();
        paging_map_page(proc->page_dir, stack_top - (i + 1) * PAGE_SIZE, frame,
                       PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
    }
    
    proc->eip = elf_header->e_entry;
    proc->esp = stack_top;
    proc->ebp = stack_top;
    proc->eflags = 0x202;
    
    return proc;
}

void process_switch_to(struct process* proc) {
    if (!proc || proc->state == PROCESS_STATE_TERMINATED) {
        return;
    }
    
    current_process = proc;
    proc->state = PROCESS_STATE_RUNNING;
    
    paging_switch_directory(proc->page_dir);
    
    process_enter_usermode(proc->eip, proc->esp);
}

void process_exit() {
    if (current_process) {
        current_process->state = PROCESS_STATE_TERMINATED;
        paging_destroy_address_space(current_process->page_dir);
        pmm_free_frame((uint32_t)current_process);
        current_process = 0;
    }
}
