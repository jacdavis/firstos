#include "pmm.h"

#define FRAME_SIZE 4096
#define FRAMES_PER_BYTE 8

extern uint32_t _kernel_end;
static uint32_t* frame_bitmap = 0;
static uint32_t total_frames = 0;
static uint32_t bitmap_size = 0;

void pmm_init(uint32_t mem_low, uint32_t mem_high) {
    total_frames = mem_high / FRAME_SIZE;
    bitmap_size = total_frames / FRAMES_PER_BYTE;
    
    frame_bitmap = (uint32_t*)&_kernel_end;
    
    for (uint32_t i = 0; i < bitmap_size / 4; i++) {
        frame_bitmap[i] = 0xFFFFFFFF;
    }
    
    uint32_t kernel_frames = ((uint32_t)&_kernel_end + bitmap_size * 4) / FRAME_SIZE + 1;
    for (uint32_t i = 0; i < kernel_frames; i++) {
        uint32_t byte = i / 32;
        uint32_t bit = i % 32;
        frame_bitmap[byte] &= ~(1 << bit);
    }
}

uint32_t pmm_alloc_frame() {
    for (uint32_t i = 0; i < bitmap_size / 4; i++) {
        if (frame_bitmap[i] != 0) {
            for (uint32_t j = 0; j < 32; j++) {
                if (frame_bitmap[i] & (1 << j)) {
                    frame_bitmap[i] &= ~(1 << j);
                    return (i * 32 + j) * FRAME_SIZE;
                }
            }
        }
    }
    return 0;
}

void pmm_free_frame(uint32_t frame) {
    uint32_t frame_num = frame / FRAME_SIZE;
    uint32_t byte = frame_num / 32;
    uint32_t bit = frame_num % 32;
    frame_bitmap[byte] |= (1 << bit);
}
