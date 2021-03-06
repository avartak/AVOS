#include <kernel/arch/processor/include/idt.h>

void IDT_SetupEntry(struct IDT_Entry* entry, uintptr_t address, uint16_t segment, uint8_t type) {

    entry->addr_low       = (uint16_t) (address & 0x0000FFFF);
    entry->segment        = segment;
    entry->zero           = 0x00;
    entry->type_attr      = type;
    entry->addr_high      = (uint16_t)((address & 0xFFFF0000) >> 16);

    return;

}

