#include <kernel/include/machine.h>
#include <x86/kernel/include/physmem.h>
#include <x86/drivers/include/pic.h>

void Interrupt_End(uint8_t interrupt) {

    if (interrupt >= PIC_IRQ_OFFSET && interrupt < PIC_IRQ_OFFSET + PIC_NUM_IRQS) {
        PIC_SendEOI(interrupt - PIC_IRQ_OFFSET);
    }

}

void Interrupt_Enable(uint8_t interrupt) {

    if (interrupt >= PIC_IRQ_OFFSET && interrupt < PIC_IRQ_OFFSET + PIC_NUM_IRQS) {
		PIC_EnableInterrupt(interrupt - PIC_IRQ_OFFSET);
    }

}

void Interrupt_Disable(uint8_t interrupt) {

    if (interrupt >= PIC_IRQ_OFFSET && interrupt < PIC_IRQ_OFFSET + PIC_NUM_IRQS) {
        PIC_DisableInterrupt(interrupt - PIC_IRQ_OFFSET);
    }

}


bool Memory_AllocateBlock(uintptr_t virtual_address) {

	return Physical_Memory_AllocatePage(virtual_address);

}

bool Memory_FreeBlock(uintptr_t virtual_address) {

	return Physical_Memory_FreePage(virtual_address);

}
