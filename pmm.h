#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void pmm_init(uint32_t mem_low, uint32_t mem_high);
uint32_t pmm_alloc_frame();
void pmm_free_frame(uint32_t frame);

#ifdef __cplusplus
}
#endif

#endif
