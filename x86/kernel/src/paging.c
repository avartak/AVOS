#include <x86/kernel/include/defs.h>
#include <x86/kernel/include/paging.h>

static inline void EnablePGBitInCR0() {

    asm volatile (
        " \
        movl %%cr0, %%eax;     \
        or $0x80000000, %%eax; \
        movl %%eax, %%cr0;     \
        "
        :
        :
        : "%eax"
    )

}

static inline void LoadPageDirectory(uint32_t* pdt) {

    asm volatile (
        " \
        movl %0, %%eax;    \
        movl %%eax, %%cr3; \
        "
        :
        : "m"(*pdt)
        : "%eax"
    )

}

void InitPaging() {

    // We will now clear the identity map
    uint32_t* page_directory = (uint32_t*)LOC_PAGE_DIRECTORY_VM;
    page_directory[0] = 0;

    // We will reserve physical memory 4 MB - 8 MB for page tables and map it to higher half ; The corresponding page table is put right after the kernel page table in memory (at 0x12000)
    uint32_t* page_map = (uint32_t*)LOC_PAGEMAP_TABLE_VM;

    uint32_t i;
    for(i = 0; i < 1024; i++) {
        page_map[i] = (i * 0x1000 + LOC_PAGEMAP_PM) | 3; // attributes: supervisor level, read/write, present.
    }
    page_directory[769] = ((uint32_t)page_map) | 3;

}

