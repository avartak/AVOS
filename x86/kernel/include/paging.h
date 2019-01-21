#ifndef X86_KERNEL_PAGING_H
#define X86_KERNEL_PAGING_H

#include <stdint.h>

#ifdef  BINARY_IMAGE
#define LOC_KERNEL_HH_OFFSET  0xC0000000
#else
#define LOC_KERNEL_HH_OFFSET  0
#endif

#define LOC_PAGEMAP_PM        0x400000

#define asm __asm__
#define volatile __volatile__

static inline void EnablePGBitInCR0();
static inline void LoadPageDirectory(uintptr_t pdt);

extern uint32_t page_directory[]__attribute__((aligned(0x1000)));

inline void EnablePGBitInCR0() {

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

inline void LoadPageDirectory(uintptr_t pdt) {

    asm volatile (
        " \
        movl %0, %%eax;    \
        movl %%eax, %%cr3; \
        "
        :
        : "m"(pdt)
        : "%eax"
    );

}

extern void AddPageTableToDirectory     (uintptr_t pt, uint32_t  entry, uint16_t attr);
extern void MapPageTableTo4MBMemoryChunk(uintptr_t pt, uintptr_t addr , uint16_t attr);

extern void InitPaging();

#endif
