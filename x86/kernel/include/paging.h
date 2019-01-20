#ifndef X86_KERNEL_PAGING_H
#define X86_KERNEL_PAGING_H

#include <stdint.h>

#define LOC_PAGE_DIRECTORY_VM 0xC0010000
#define LOC_KERNEL_TABLE_VM   0xC0011000
#define LOC_PAGEMAP_TABLE_VM  0xC0012000
#define LOC_PAGEMAP_VM        0xC0400000
#define LOC_PAGEMAP_PM        0x400000

static inline void EnablePGBitInCR0();
static inline void LoadPageDirectory(uint32_t* pdt);

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

inline void LoadPageDirectory(uint32_t* pdt) {

    asm volatile (
        " \
        movl %0, %%eax;    \
        movl %%eax, %%cr3; \
        "
        :
        : "m"(*pdt)
        : "%eax"
    );

}

extern void InitPaging();

#endif
