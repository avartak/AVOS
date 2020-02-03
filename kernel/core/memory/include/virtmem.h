#ifndef KERNEL_VIRTMEM_H
#define KERNEL_VIRTMEM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

extern bool      Memory_IsPageMapped(uintptr_t* page_dir, uintptr_t virt_addr);
extern bool      Memory_MapPage(uintptr_t* page_dir, uintptr_t virt_addr, uintptr_t phys_addr, uint16_t flags, bool create);
extern bool      Memory_UnmapPage(uintptr_t* page_dir, uintptr_t virt_addr);
extern size_t    Memory_MapPages(uintptr_t* page_dir, void* virt_addr, size_t size, uint16_t flags);
extern bool      Memory_UnmapPages(uintptr_t* page_dir, void* virt_addr, size_t size);
extern bool      Memory_MakePageDirectory(uintptr_t* page_dir);
extern void      Memory_UnmakePageDirectory(uintptr_t* page_dir);
extern bool      Memory_EnablePageFlags(uintptr_t* page_dir, void* virt_addr, uint16_t flags);
extern bool      Memory_DisablePageFlags(uintptr_t* page_dir, void* virt_addr, uint16_t flags);
extern bool      Memory_ClonePageDirectory(uintptr_t* page_dir, uintptr_t* clone);
extern uintptr_t Memory_GetHigherHalfAddress(uintptr_t* page_dir, uintptr_t user_addr); 
extern bool      Memory_Copy(uintptr_t* dest_page_dir, uintptr_t dest_addr, uintptr_t src_addr, size_t nbytes);

#endif
