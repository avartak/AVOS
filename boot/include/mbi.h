#ifndef BOOT_MBI_H
#define BOOT_MBI_H

#include <boot/include/defs.h>
#include <boot/include/multiboot.h>

extern bool  Multiboot_SaveBootLoaderInfo (uintptr_t mbi_addr);
extern bool  Multiboot_SaveBootCommand    (uintptr_t mbi_addr);
extern bool  Multiboot_SaveMemoryMaps     (uintptr_t mbi_addr);
extern bool  Multiboot_SaveBasicMemoryInfo(uintptr_t mbi_addr);
extern bool  Multiboot_SaveAPMInfo        (uintptr_t mbi_addr);
extern bool  Multiboot_SaveSMBIOSInfo     (uintptr_t mbi_addr);
extern bool  Multiboot_SaveACPIInfo       (uintptr_t mbi_addr, bool old);
extern bool  Multiboot_SaveBootDeviceInfo (uintptr_t mbi_addr, struct Boot_KernelInfo* kernel_info);
extern bool  Multiboot_SaveGraphicsInfo   (uintptr_t mbi_addr, struct Boot_KernelInfo* kernel_info);
extern bool  Multiboot_SaveLoadBaseAddress(uintptr_t mbi_addr, struct Boot_KernelInfo* kernel_info);
extern bool  Multiboot_SaveInfo           (uintptr_t mbi_addr, struct Boot_KernelInfo* kernel_info);

#endif
