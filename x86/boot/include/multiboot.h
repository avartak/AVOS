#ifndef X86_BOOT_MULTIBOOT_H
#define X86_BOOT_MULTIBOOT_H

#include <kernel/include/common.h>
#include <kernel/include/multiboot.h>
#include <csupport/include/string.h>

extern size_t    Multiboot_LoadKernel            (uintptr_t image, size_t size, uintptr_t start_addr, uintptr_t mbi_addr);
extern uintptr_t Multiboot_GetHeader             (uintptr_t start_addr, size_t size);
extern uintptr_t Multiboot_GetKernelEntry        (uintptr_t start_addr, uintptr_t multiboot_header_ptr);
extern bool      Multiboot_CheckForValidMBI      (uintptr_t mbi_addr);
extern bool      Multiboot_CreateMBI             (uintptr_t mbi_addr);
extern uintptr_t Multiboot_FindMBITagAddress     (uintptr_t mbi_addr, uint32_t tag_type);
extern bool      Multiboot_SaveBootLoaderInfo    (uintptr_t mbi_addr);
extern bool      Multiboot_SaveMemoryInfo        (uintptr_t mbi_addr);
extern bool      Multiboot_SaveGraphicsInfo      (uintptr_t mbi_addr, uintptr_t multiboot_header_addr);
extern bool      Multiboot_SaveELFSectionHeaders (uintptr_t mbi_addr, uintptr_t image);
extern bool      Multiboot_CheckForSupportFailure(uintptr_t multiboot_header_ptr);

#endif
