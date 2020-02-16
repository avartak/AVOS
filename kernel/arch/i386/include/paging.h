#ifndef KERNEL_X86_PAGING_H
#define KERNEL_X86_PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define X86_PAGING_PAGESIZE           0x1000
#define X86_PAGING_EXTPAGESIZE        0x400000
#define X86_PAGING_PGDIR_NENTRIES     0x400
#define X86_PAGING_PGTAB_NENTRIES     0x400

#define X86_PAGING_PDE_PRESENT        1
#define X86_PAGING_PDE_READWRITE      2
#define X86_PAGING_PDE_USER           4
#define X86_PAGING_PDE_WRITETHROUGH   8
#define X86_PAGING_PDE_CACHEDISABLE   0x10
#define X86_PAGING_PDE_ACCESSED       0x20
#define X86_PAGING_PDE_PSE            0x80

#define X86_PAGING_PTE_PRESENT        1
#define X86_PAGING_PTE_READWRITE      2
#define X86_PAGING_PTE_USER           4
#define X86_PAGING_PTE_WRITETHROUGH   8
#define X86_PAGING_PTE_CACHEDISABLE   0x10
#define X86_PAGING_PTE_ACCESSED       0x20
#define X86_PAGING_PTE_DIRTY          0x40
#define X86_PAGING_PTE_GLOBAL         0x100

#define X86_PAGING_PGDIR_IDX(x)       ((uint32_t)(x) >> 22) & 0x3FF
#define X86_PAGING_PGTAB_IDX(x)       ((uint32_t)(x) >> 12) & 0x3FF

#define X86_PAGING_ROUNDUPPAGE(x)     (((uint32_t)(x)+X86_PAGING_PAGESIZE−1) & ~(X86_PAGING_PAGESIZE−1))
#define X86_PAGING_ROUNDDOWNPAGE(x)   ( (uint32_t)(x)                        & ~(X86_PAGING_PAGESIZE−1))


extern bool      Paging_IsPageMapped(uintptr_t* page_dir, uintptr_t virt_addr);
extern bool      Paging_MapPage(uintptr_t* page_dir, uintptr_t virt_addr, uintptr_t phys_addr, bool create);
extern bool      Paging_UnmapPage(uintptr_t* page_dir, uintptr_t virt_addr);
extern size_t    Paging_MapPages(uintptr_t* page_dir, void* virt_addr, size_t size);
extern bool      Paging_UnmapPages(uintptr_t* page_dir, void* virt_addr, size_t size);
extern bool      Paging_MakePageDirectory(uintptr_t* page_dir);
extern void      Paging_UnmakePageDirectory(uintptr_t* page_dir);
extern bool      Paging_ClonePageDirectory(uintptr_t* page_dir, uintptr_t* clone);
extern uintptr_t Paging_GetHigherHalfAddress(uintptr_t* page_dir, uintptr_t user_addr);
extern bool      Paging_Copy(uintptr_t* dest_page_dir, uintptr_t dest_addr, uintptr_t src_addr, size_t nbytes);

#endif
