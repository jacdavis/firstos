#include "paging.h"
#include "pmm.h"

struct page_directory* kernel_directory = 0;
struct page_directory* current_directory = 0;

extern "C" void paging_load_directory(uint32_t* dir);
extern "C" void paging_enable();

extern uint32_t _kernel_end;

void paging_init() {
    uint32_t pd_addr = ((uint32_t)&_kernel_end + 0xFFF) & 0xFFFFF000;
    kernel_directory = (struct page_directory*)pd_addr;
    
    for (int i = 0; i < 1024; i++) {
        kernel_directory->tables[i] = 0;
    }
    
    uint32_t table_addr = pd_addr + 0x1000;
    for (uint32_t vaddr = 0; vaddr < 0x400000; vaddr += PAGE_SIZE) {
        uint32_t dir_idx = vaddr >> 22;
        uint32_t table_idx = (vaddr >> 12) & 0x3FF;
        
        if (!(kernel_directory->tables[dir_idx] & PAGE_PRESENT)) {
            kernel_directory->tables[dir_idx] = table_addr | PAGE_PRESENT | PAGE_WRITE;
            uint32_t* table = (uint32_t*)table_addr;
            for (int j = 0; j < 1024; j++) {
                table[j] = 0;
            }
            table_addr += 0x1000;
        }
        
        uint32_t* table = (uint32_t*)(kernel_directory->tables[dir_idx] & 0xFFFFF000);
        table[table_idx] = vaddr | PAGE_PRESENT | PAGE_WRITE;
    }
    
    current_directory = kernel_directory;
    paging_load_directory((uint32_t*)kernel_directory);
    paging_enable();
}

struct page_directory* paging_create_address_space() {
    struct page_directory* dir = (struct page_directory*)pmm_alloc_frame();
    
    for (int i = 0; i < 1024; i++) {
        dir->tables[i] = 0;
    }
    
    for (int i = 0; i < 256; i++) {
        dir->tables[i] = kernel_directory->tables[i];
    }
    
    return dir;
}

void paging_destroy_address_space(struct page_directory* dir) {
    if (dir == kernel_directory) return;
    
    for (int i = 256; i < 1024; i++) {
        if (dir->tables[i] & PAGE_PRESENT) {
            uint32_t table_frame = dir->tables[i] & 0xFFFFF000;
            uint32_t* table = (uint32_t*)(table_frame);
            
            for (int j = 0; j < 1024; j++) {
                if (table[j] & PAGE_PRESENT) {
                    uint32_t frame = table[j] & 0xFFFFF000;
                    pmm_free_frame(frame);
                }
            }
            pmm_free_frame(table_frame);
        }
    }
    
    pmm_free_frame((uint32_t)dir);
}

void paging_map_page(struct page_directory* dir, uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t dir_idx = virt >> 22;
    uint32_t table_idx = (virt >> 12) & 0x3FF;
    
    uint32_t* table;
    if (!(dir->tables[dir_idx] & PAGE_PRESENT)) {
        uint32_t table_frame = pmm_alloc_frame();
        dir->tables[dir_idx] = table_frame | PAGE_PRESENT | PAGE_WRITE | (flags & PAGE_USER);
        table = (uint32_t*)table_frame;
        for (int i = 0; i < 1024; i++) {
            table[i] = 0;
        }
    } else {
        table = (uint32_t*)(dir->tables[dir_idx] & 0xFFFFF000);
    }
    
    table[table_idx] = (phys & 0xFFFFF000) | flags;
}

void paging_switch_directory(struct page_directory* dir) {
    current_directory = dir;
    paging_load_directory((uint32_t*)dir);
}

uint32_t paging_get_physical_address(struct page_directory* dir, uint32_t virt) {
    uint32_t dir_idx = virt >> 22;
    uint32_t table_idx = (virt >> 12) & 0x3FF;
    
    if (!(dir->tables[dir_idx] & PAGE_PRESENT)) {
        return 0;
    }
    
    uint32_t* table = (uint32_t*)(dir->tables[dir_idx] & 0xFFFFF000);
    
    if (!(table[table_idx] & PAGE_PRESENT)) {
        return 0;
    }
    
    return (table[table_idx] & 0xFFFFF000) | (virt & 0xFFF);
}
