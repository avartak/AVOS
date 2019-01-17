#ifndef X86_KERNEL_PAGING_H
#define X86_KERNEL_PAGING_H

#include <stdint.h>

#define LOC_PAGE_DIRECTORY_VM 0xC0010000
#define LOC_KERNEL_TABLE_VM   0xC0011000
#define LOC_PAGEMAP_TABLE_VM  0xC0012000
#define LOC_PAGEMAP_VM        0xC0400000
#define LOC_PAGEMAP_PM        0x400000

extern inline void EnablePGBitInCR0();
extern inline void LoadPageDirectory(uint32_t* pdt);
extern void InitPaging();

#endif
