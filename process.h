#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "paging.h"

#define PROCESS_STATE_READY 0
#define PROCESS_STATE_RUNNING 1
#define PROCESS_STATE_TERMINATED 2

struct process {
    uint32_t pid;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    uint32_t eflags;
    struct page_directory* page_dir;
    uint32_t state;
};

#ifdef __cplusplus
extern "C" {
#endif

void process_init();
struct process* process_create_from_elf(void* elf_data, uint32_t size);
void process_switch_to(struct process* proc);
void process_exit();

extern struct process* current_process;

#ifdef __cplusplus
}
#endif

#endif
