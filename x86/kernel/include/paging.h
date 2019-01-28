#ifndef X86_KERNEL_PAGING_H
#define X86_KERNEL_PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PAGING_KERN_TABLE_FLAGS 0x3
#define PAGING_KERN_PAGE_FLAGS  0x3
#define PAGING_USER_TABLE_FLAGS 0x7
#define PAGING_USER_PAGE_FLAGS  0x7

#define PAGING_NULL_PTR         0xFFFFFFFF

#define asm __asm__
#define volatile __volatile__

extern uint32_t Paging_kernel_directory[]__attribute__((aligned(0x1000)));
extern uint32_t Paging_kernel_selftable[]__attribute__((aligned(0x1000)));

static inline void     Paging_EnablePGBitInCR0();
static inline void     Paging_LoadDirectory           (uintptr_t pd);
static inline void     Paging_MapTableInDirectory     (uint32_t* pd, uintptr_t pt, uint32_t entry, uint16_t attr);
static inline void     Paging_MapPageInTable          (uint32_t* pt, uintptr_t pg, uint32_t entry, uint16_t attr);
static inline void     Paging_MapMemoryBlockInTable   (uint32_t* pt, uintptr_t addr, uint16_t attr);
static inline uint32_t Paging_GetDirectoryEntry       (uintptr_t virtual_address);
static inline uint32_t Paging_GetTableEntry           (uintptr_t virtual_address);

extern uintptr_t       Paging_GetPhysicalAddress      (uintptr_t virtual_address);
extern bool            Paging_UnmapVirtualPage        (uintptr_t virtual_address);
extern bool            Paging_MapVirtualToPhysicalPage(uintptr_t virtual_address, uintptr_t phys_address, uint16_t attr);
extern void            Paging_Initialize();



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
    asm volatile ("movl %%eax, %%cr3;" : : "a"(pd) : );
}

inline void Paging_MapTableInDirectory(uint32_t* pd, uintptr_t pt, uint32_t entry, uint16_t attr) {
    pd[entry & 0x03FF] = (pt & 0xFFFFF000) | (attr & 0x0FFF);
}

inline void Paging_MapPageInTable(uint32_t* pt, uintptr_t pg, uint32_t entry, uint16_t attr) {
    pt[entry & 0x03FF] = (pg & 0xFFFFF000) | (attr & 0x0FFF);
}

inline void Paging_MapMemoryBlockInTable(uint32_t* pt, uintptr_t addr, uint16_t attr) {
    for(size_t i = 0; i < 1024; i++) pt[i] = i * 0x1000 + ((addr & 0xFFFFF000) | (attr & 0x0FFF));
}

inline uint32_t Paging_GetDirectoryEntry(uintptr_t virtual_address) {
	return (uint32_t)virtual_address >> 22;
}

inline uint32_t Paging_GetTableEntry(uintptr_t virtual_address) {
	return (uint32_t)virtual_address >> 12 & 0x03FF;
}

#endif
