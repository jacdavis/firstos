#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR 1024

#define PAGE_PRESENT    0x1
#define PAGE_WRITE      0x2
#define PAGE_USER       0x4

struct page_directory {
    uint32_t tables[1024];
};

#ifdef __cplusplus
extern "C" {
#endif

void paging_init();
struct page_directory* paging_create_address_space();
void paging_destroy_address_space(struct page_directory* dir);
void paging_map_page(struct page_directory* dir, uint32_t virt, uint32_t phys, uint32_t flags);
void paging_switch_directory(struct page_directory* dir);
uint32_t paging_get_physical_address(struct page_directory* dir, uint32_t virt);

extern struct page_directory* kernel_directory;
extern struct page_directory* current_directory;

#ifdef __cplusplus
}
#endif

#endif
