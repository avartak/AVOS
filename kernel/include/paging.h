#ifndef KERNEL_PAGING_H
#define KERNEL_PAGING_H

#include <kernel/include/common.h>
#include <kernel/include/process.h>

#define PAGING_KERN_TABLE_FLAGS 0x3
#define PAGING_KERN_PAGE_FLAGS  0x3
#define PAGING_USER_TABLE_FLAGS 0x7
#define PAGING_USER_PAGE_FLAGS  0x7

extern uint32_t        Paging_directory[]__attribute__((aligned(0x1000)));

extern void            Paging_LoadDirectory     (uintptr_t pd);
extern void            Paging_MapEntry          (uint32_t* table, uintptr_t entry_ptr, uint32_t entry_num, uint16_t attr);
extern uint32_t        Paging_GetDirectoryEntry (uintptr_t virtual_address);
extern uint32_t        Paging_GetTableEntry     (uintptr_t virtual_address);

extern bool            Paging_TableExists       (uintptr_t virtual_address);
extern bool            Paging_ClearTable        (uintptr_t virtual_address);
extern uintptr_t       Paging_GetPhysicalAddress(uintptr_t virtual_address);
extern bool            Paging_UnmapVirtualPage  (uintptr_t virtual_address);
extern bool            Paging_MapVirtualPage    (uintptr_t virtual_address, uintptr_t phys_address, uint16_t attr);
extern bool            Paging_SetupDirectory    (struct Process* proc);

#endif
