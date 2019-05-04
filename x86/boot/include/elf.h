#ifndef X86_BOOT_ELF_H
#define X86_BOOT_ELF_H

#include <kernel/include/common.h>
#include <kernel/include/elf.h>

extern bool      Elf32_IsValidELF              (uintptr_t image);
extern bool      Elf32_IsValidExecutable       (uintptr_t image);
extern bool      Elf32_IsValidRelocatable      (uintptr_t image);
extern bool      Elf32_IsValidiStaticExecutable(uintptr_t image);
extern size_t    Elf32_LoadStaticExecutable    (uintptr_t image, uintptr_t start_addr);
extern size_t    Elf32_LoadSectionHeaderTable  (uintptr_t image, uintptr_t start_addr, bool load_extra);
extern size_t    Elf32_LoadBSSLikeSections     (uintptr_t image, uintptr_t start_addr);
extern bool      Elf32_Relocate                (uintptr_t image);
extern bool      Elf32_RelocateSymbol          (uintptr_t image, Elf32_Rel* rel, Elf32_Shdr* rel_table_hdr);
extern uintptr_t Elf32_GetSymbolValue          (uintptr_t image, uintptr_t table, size_t idx);
extern void*     Elf32_LookupSymbol(const char* name);

#endif
