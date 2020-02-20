#include <kernel/arch/processor/include/gdt.h>

void X86_GDT_SetupEntry(struct X86_GDT_Entry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {

    entry->limit_low      = (uint16_t)  (limit & 0x0000FFFF);
    entry->base_low       = (uint16_t)  (base  & 0x0000FFFF);
    entry->base_middle    = (uint8_t ) ((base  & 0x00FF0000) >> 16);
    entry->access         = access;
    entry->limit_flags    = (uint8_t ) ((limit & 0x000F0000) >> 16);
    entry->limit_flags   += (uint8_t ) ((flags & 0x0F)       << 4);
    entry->base_high      = (uint8_t ) ((base  & 0xFF000000) >> 24);

    return;

}

