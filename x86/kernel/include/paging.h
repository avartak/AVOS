#ifndef X86_KERNEL_PAGING_H
#define X86_KERNEL_PAGING_H

#include <stdint.h>

#define LOC_PAGEMAP_PM 0x400000

#define asm __asm__
#define volatile __volatile__


extern uint32_t Paging_directory[]__attribute__((aligned(0x1000)));

static inline void Paging_EnablePGBitInCR0();
static inline void Paging_LoadDirectory(uintptr_t pd);

inline void Paging_EnablePGBitInCR0() {

    asm volatile (
        " \
        movl %%cr0, %%eax;     \
        or $0x80000000, %%eax; \
        movl %%eax, %%cr0;     \
        "
        :
        :
        : "%eax"
    );

}

inline void Paging_LoadDirectory(uintptr_t pd) {

    asm volatile (
        " \
        movl %0, %%eax;    \
        movl %%eax, %%cr3; \
        "
        :
        : "m"(pd)
        : "%eax"
    );

}

extern void Paging_MapTableInDirectory  (uintptr_t pd, uintptr_t pt, uint32_t entry, uint16_t attr);
extern void Paging_MapPageInTable       (uintptr_t pt, uintptr_t pg, uint32_t entry, uint16_t attr);
extern void Paging_MapMemoryBlockInTable(              uintptr_t pt, uintptr_t addr, uint16_t attr);

extern void Paging_Initialize();

#endif
