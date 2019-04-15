#ifndef X86_BOOT_BOOT_INFO_H
#define X86_BOOT_BOOT_INFO_H

#include <kernel/include/common.h>
#include <x86/boot/include/vbe.h>
#include <x86/boot/include/e820.h>
#include <x86/boot/include/ram.h>

struct Boot_Info_Entry {
    uintptr_t address;
    uint32_t  size;
}__attribute__((packed));

extern struct Boot_Info_Entry Boot_Info;
extern struct VBE_Info VBE_Table;

extern bool Boot_Info_Prepare();

#endif
