#include <x86/kernel/include/idt.h>

struct IDT_Entry      IDT_entries[0x100];
struct IDT_Descriptor IDT_desc;

void IDT_SetupEntry(struct IDT_Entry* entry, uintptr_t address, uint16_t segment, uint8_t type) {

	entry->addr_low       = (uint16_t) (address & 0x0000FFFF);
	entry->segment        = segment;
	entry->zero           = 0x00;
	entry->type_attr      = type;
	entry->addr_high      = (uint16_t)((address & 0xFFFF0000) >> 16);

    return;

}

void IDT_Initialize() {

	for (size_t i = 0; i < 256; i++) IDT_SetupEntry(&(IDT_entries[i]), 0x00000000, 0x00, 0x00);

	uint8_t interrupt_type = (IDT_INT_ACCESS_KERN << 4) | IDT_INT_TYPE_INTERRUPT;

    IDT_SetupEntry(&(IDT_entries[0x00] ), (uintptr_t)Interrupt_0 , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x01] ), (uintptr_t)Interrupt_1 , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x02] ), (uintptr_t)Interrupt_2 , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x03] ), (uintptr_t)Interrupt_3 , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x04] ), (uintptr_t)Interrupt_4 , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x05] ), (uintptr_t)Interrupt_5 , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x06] ), (uintptr_t)Interrupt_6 , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x07] ), (uintptr_t)Interrupt_7 , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x08] ), (uintptr_t)Interrupt_8 , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x09] ), (uintptr_t)Interrupt_9 , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x0A] ), (uintptr_t)Interrupt_A , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x0B] ), (uintptr_t)Interrupt_B , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x0C] ), (uintptr_t)Interrupt_C , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x0D] ), (uintptr_t)Interrupt_D , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x0E] ), (uintptr_t)Interrupt_E , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x0F] ), (uintptr_t)Interrupt_F , IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x10] ), (uintptr_t)Interrupt_10, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x11] ), (uintptr_t)Interrupt_11, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x12] ), (uintptr_t)Interrupt_12, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x13] ), (uintptr_t)Interrupt_13, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x14] ), (uintptr_t)Interrupt_14, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x15] ), (uintptr_t)Interrupt_15, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x16] ), (uintptr_t)Interrupt_16, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x17] ), (uintptr_t)Interrupt_17, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x18] ), (uintptr_t)Interrupt_18, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x19] ), (uintptr_t)Interrupt_19, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x1A] ), (uintptr_t)Interrupt_1A, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x1B] ), (uintptr_t)Interrupt_1B, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x1C] ), (uintptr_t)Interrupt_1C, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x1D] ), (uintptr_t)Interrupt_1D, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x1E] ), (uintptr_t)Interrupt_1E, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x1F] ), (uintptr_t)Interrupt_1F, IDT_KERN_CODE_SEG, interrupt_type);

    IDT_SetupEntry(&(IDT_entries[0x20] ), (uintptr_t)Interrupt_20, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x21] ), (uintptr_t)Interrupt_21, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x22] ), (uintptr_t)Interrupt_22, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x23] ), (uintptr_t)Interrupt_23, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x24] ), (uintptr_t)Interrupt_24, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x25] ), (uintptr_t)Interrupt_25, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x26] ), (uintptr_t)Interrupt_26, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x27] ), (uintptr_t)Interrupt_27, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x28] ), (uintptr_t)Interrupt_28, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x29] ), (uintptr_t)Interrupt_29, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x2A] ), (uintptr_t)Interrupt_2A, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x2B] ), (uintptr_t)Interrupt_2B, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x2C] ), (uintptr_t)Interrupt_2C, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x2D] ), (uintptr_t)Interrupt_2D, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x2E] ), (uintptr_t)Interrupt_2E, IDT_KERN_CODE_SEG, interrupt_type);
    IDT_SetupEntry(&(IDT_entries[0x2F] ), (uintptr_t)Interrupt_2F, IDT_KERN_CODE_SEG, interrupt_type);

    IDT_SetupEntry(&(IDT_entries[0x80] ), (uintptr_t)Interrupt_80, IDT_KERN_CODE_SEG, interrupt_type);

    IDT_desc.limit = (sizeof(struct IDT_Entry))*0x100 - 1;
    IDT_desc.base  = (uintptr_t)IDT_entries;
   
    IDT_Load(&IDT_desc);

    return;

}



