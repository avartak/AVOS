#ifndef KERNEL_PAGING_H
#define KERNEL_PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PAGING_PAGESIZE           0x1000
#define PAGING_EXTPAGESIZE        0x400000
#define PAGING_PGDIR_NENTRIES     0x400
#define PAGING_PGTAB_NENTRIES     0x400

#define PAGING_PDE_SIZE           0x400000
#define PAGING_PDE_PRESENT        1
#define PAGING_PDE_READWRITE      2
#define PAGING_PDE_USER           4
#define PAGING_PDE_WRITETHROUGH   8
#define PAGING_PDE_CACHEDISABLE   0x10
#define PAGING_PDE_ACCESSED       0x20
#define PAGING_PDE_PSE            0x80

#define PAGING_PTE_SIZE           0x1000
#define PAGING_PTE_PRESENT        1
#define PAGING_PTE_READWRITE      2
#define PAGING_PTE_USER           4
#define PAGING_PTE_WRITETHROUGH   8
#define PAGING_PTE_CACHEDISABLE   0x10
#define PAGING_PTE_ACCESSED       0x20
#define PAGING_PTE_DIRTY          0x40
#define PAGING_PTE_GLOBAL         0x100

#define PAGING_PGDIR_IDX(x)       (((uintptr_t)(x) >> 22) & 0x3FF)
#define PAGING_PGTAB_IDX(x)       (((uintptr_t)(x) >> 12) & 0x3FF)

#define PAGING_ROUNDUPPAGE(x)     (((uintptr_t)(x)+PAGING_PAGESIZE−1) & ~(PAGING_PAGESIZE−1))
#define PAGING_ROUNDDOWNPAGE(x)   ( (uintptr_t)(x)                    & ~(PAGING_PAGESIZE−1))

struct Process;

extern bool       Paging_IsPageMapped(struct Process* proc, uintptr_t virt_addr);
extern bool       Paging_MapPage(struct Process* proc, uintptr_t virt_addr, uintptr_t phys_addr, bool create);
extern bool       Paging_UnmapPage(struct Process* proc, uintptr_t virt_addr);
extern size_t     Paging_MapPages(struct Process* proc, void* virt_addr, size_t size);
extern bool       Paging_UnmapPages(struct Process* proc, void* virt_addr, size_t size);
extern bool       Paging_Initialize(struct Process* proc);
extern void       Paging_Terminate(struct Process* proc);
extern bool       Paging_SetFlags(struct Process* proc, void* virt_addr, uint16_t flags);
extern bool       Paging_UnsetFlags(struct Process* proc, void* virt_addr, uint16_t flags);
extern bool       Paging_Clone(struct Process* proc, struct Process* clone_proc);
extern uintptr_t  Paging_GetHigherHalfAddress(struct Process* proc, uintptr_t user_addr);
extern bool       Paging_Copy(struct Process* proc, uintptr_t dest_addr, uintptr_t src_addr, size_t nbytes);

#endif
