#include <kernel/include/machine.h>
#include <kernel/include/interrupts.h>
#include <kernel/include/physmem.h>
#include <x86/drivers/include/pic.h>

void Interrupt_End(uint8_t interrupt) {

    if (interrupt >= PIC_IRQ_OFFSET && interrupt < PIC_IRQ_OFFSET + PIC_NUM_IRQS) {
        PIC_SendEOI(interrupt - PIC_IRQ_OFFSET);
    }

}

void Interrupt_Enable(uint8_t interrupt) {

	uint16_t irq_bit = 1 << (interrupt-PIC_IRQ_OFFSET);
    if (interrupt >= PIC_IRQ_OFFSET && interrupt < PIC_IRQ_OFFSET + PIC_NUM_IRQS) {
		PIC_EnableInterrupt(interrupt - PIC_IRQ_OFFSET);
		Interrupt_active_IRQs |= irq_bit;
    }

}

void Interrupt_Disable(uint8_t interrupt) {

	uint16_t irq_bit = 1 << (interrupt-PIC_IRQ_OFFSET);
    if (interrupt >= PIC_IRQ_OFFSET && interrupt < PIC_IRQ_OFFSET + PIC_NUM_IRQS) {
        PIC_DisableInterrupt(interrupt - PIC_IRQ_OFFSET);
		Interrupt_active_IRQs &= (~irq_bit);
    }

}

