#ifndef X86_BOOT_BOOTINFO_H
#define X86_BOOT_BOOTINFO_H

#include <kernel/include/common.h>
#include <x86/boot/include/vbe.h>
#include <x86/boot/include/e820.h>
#include <x86/boot/include/ram.h>

extern void BootInfo_Store(uintptr_t info_ptr, uintptr_t tables_addr);

#endif
