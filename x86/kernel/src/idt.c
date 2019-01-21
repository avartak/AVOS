#include <x86/kernel/include/pic.h>
#include <x86/kernel/include/idt.h>
#include <x86/kernel/include/interrupts.h>

#include <stddef.h>

struct IDTEntry idt[0x100];
struct IDTRecord idtr;

void SetupIDTEntry(struct IDTEntry* entry, uintptr_t address, uint16_t segment, uint8_t type) {

	entry->addr_low       = (uint16_t) (address & 0x0000FFFF);
	entry->segment        = segment;
	entry->zero           = 0x00;
	entry->type_attr      = type;
	entry->addr_high      = (uint16_t)((address & 0xFFFF0000) >> 16);

    return;

}

void SetupIDT() {

	for (size_t i = 0; i < 256; i++) SetupIDTEntry(&(idt[i]), 0x00000000, 0x00, 0x00);

	uint8_t interrupt_type = (INT_ACCESS_KERNEL << 4) | INT_TYPE_INTERRUPT;

    SetupIDTEntry(&(idt[0x00] ), (uintptr_t)Interrupt0 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x01] ), (uintptr_t)Interrupt1 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x02] ), (uintptr_t)Interrupt2 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x03] ), (uintptr_t)Interrupt3 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x04] ), (uintptr_t)Interrupt4 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x05] ), (uintptr_t)Interrupt5 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x06] ), (uintptr_t)Interrupt6 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x07] ), (uintptr_t)Interrupt7 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x08] ), (uintptr_t)Interrupt8 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x09] ), (uintptr_t)Interrupt9 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x0A] ), (uintptr_t)InterruptA , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x0B] ), (uintptr_t)InterruptB , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x0C] ), (uintptr_t)InterruptC , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x0D] ), (uintptr_t)InterruptD , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x0E] ), (uintptr_t)InterruptE , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x0F] ), (uintptr_t)InterruptF , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x10] ), (uintptr_t)Interrupt10, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x11] ), (uintptr_t)Interrupt11, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x12] ), (uintptr_t)Interrupt12, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x13] ), (uintptr_t)Interrupt13, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x14] ), (uintptr_t)Interrupt14, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x15] ), (uintptr_t)Interrupt15, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x16] ), (uintptr_t)Interrupt16, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x17] ), (uintptr_t)Interrupt17, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x18] ), (uintptr_t)Interrupt18, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x19] ), (uintptr_t)Interrupt19, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x1A] ), (uintptr_t)Interrupt1A, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x1B] ), (uintptr_t)Interrupt1B, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x1C] ), (uintptr_t)Interrupt1C, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x1D] ), (uintptr_t)Interrupt1D, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x1E] ), (uintptr_t)Interrupt1E, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x1F] ), (uintptr_t)Interrupt1F, KERNEL_CODE_SEG, interrupt_type);

    SetupIDTEntry(&(idt[0x20] ), (uintptr_t)Interrupt20, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x21] ), (uintptr_t)Interrupt21, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x22] ), (uintptr_t)Interrupt22, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x23] ), (uintptr_t)Interrupt23, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x24] ), (uintptr_t)Interrupt24, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x25] ), (uintptr_t)Interrupt25, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x26] ), (uintptr_t)Interrupt26, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x27] ), (uintptr_t)Interrupt27, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x28] ), (uintptr_t)Interrupt28, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x29] ), (uintptr_t)Interrupt29, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x2A] ), (uintptr_t)Interrupt2A, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x2B] ), (uintptr_t)Interrupt2B, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x2C] ), (uintptr_t)Interrupt2C, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x2D] ), (uintptr_t)Interrupt2D, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x2E] ), (uintptr_t)Interrupt2E, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[0x2F] ), (uintptr_t)Interrupt2F, KERNEL_CODE_SEG, interrupt_type);

    SetupIDTEntry(&(idt[0x80] ), (uintptr_t)Interrupt80, KERNEL_CODE_SEG, interrupt_type);

    idtr.limit = (sizeof(struct IDTEntry))*0x100 - 1;
    idtr.base  = (uintptr_t)idt;
   
    LoadIDT(&idtr);

    return;

}



