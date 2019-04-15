#ifndef X86_BOOT_RAM_H
#define X86_BOOT_RAM_H

#include <kernel/include/common.h>

struct RAM_Table_Entry {
    uintptr_t pointer;
    size_t    size;
}__attribute__((packed));

extern size_t RAM_Table_size;
extern struct RAM_Table_Entry* RAM_Table;

extern uintptr_t RAM_MaxPresentMemoryAddress();
extern bool      RAM_IsMemoryPresent(uintptr_t min, uintptr_t max);
extern uintptr_t RAM_StoreMap(uintptr_t addr);


#endif
