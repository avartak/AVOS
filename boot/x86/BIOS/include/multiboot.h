#ifndef X86_BOOT_MULTIBOOT_H
#define X86_BOOT_MULTIBOOT_H

#include <kernel/include/common.h>
#include <kernel/include/multiboot.h>
#include <csupport/include/string.h>
#include <boot/x86/BIOS/include/boot.h>

extern bool      Multiboot_CheckForValidMBI      (uintptr_t mbi_addr);
extern uintptr_t Multiboot_FindMBITagAddress     (uintptr_t mbi_addr, uint32_t tag_type);
extern bool      Multiboot_CreateEmptyMBI        (uintptr_t mbi_addr);
extern bool      Multiboot_TerminateTag          (uintptr_t mbi_addr, uintptr_t tag_addr);

extern uintptr_t Multiboot_GetHeader             (uintptr_t start_addr, size_t size);
extern uintptr_t Multiboot_GetKernelEntry        (uintptr_t multiboot_header_ptr);
extern bool      Multiboot_LoadKernel            (uintptr_t mbi_addr, struct Boot_Kernel_Info* kernel_info);
extern bool      Multiboot_LoadModules           (uintptr_t mbi_addr, struct Boot_Kernel_Info* kernel_info);

extern bool      Multiboot_SaveBootLoaderInfo    (uintptr_t mbi_addr);
extern bool      Multiboot_SaveBootCommand       (uintptr_t mbi_addr);
extern bool      Multiboot_SaveMemoryMaps        (uintptr_t mbi_addr);
extern bool      Multiboot_SaveBasicMemoryInfo   (uintptr_t mbi_addr);
extern bool      Multiboot_SaveBootDeviceInfo    (uintptr_t mbi_addr, uint32_t biosdev, uint32_t partition, uint32_t sub_partition);
extern bool      Multiboot_SaveGraphicsInfo      (uintptr_t mbi_addr, uintptr_t multiboot_header_addr);
extern bool      Multiboot_SaveELFSectionHeaders (uintptr_t mbi_addr, uintptr_t image);
extern bool      Multiboot_SaveAPMInfo           (uintptr_t mbi_addr);
extern bool      Multiboot_SaveSMBIOSInfo        (uintptr_t mbi_addr);
extern bool      Multiboot_SaveACPIInfo          (uintptr_t mbi_addr, bool old);
extern bool      Multiboot_SaveLoadBaseAddress   (uintptr_t mbi_addr, uintptr_t base_addr);

extern bool      Multiboot_CheckForSupportFailure(uintptr_t multiboot_header_ptr);
extern bool      Multiboot_SaveInfo              (uintptr_t mbi_addr, struct Boot_Kernel_Info* kernel_info);

#endif