#include <x86/kernel/include/io.h>
#include <x86/kernel/include/pic.h>

#include <stdint.h>

void PIC_Init(int8_t offset1, int8_t offset2) {
	uint8_t a1, a2;
	
	a1 = Inb(PIC1_DATA);                        // save masks
	a2 = Inb(PIC2_DATA);
	
	Outb(PIC1_COMD, ICW1_INIT | ICW1_ICW4);     // starts the initialization sequence (in cascade mode)
	Outb(PIC2_COMD, ICW1_INIT | ICW1_ICW4);     // ICW1_ICW4 bit says that there will be a ICW4 command word coming

	Outb(PIC1_DATA, offset1);                   // ICW2: Master PIC vector offset
	Outb(PIC2_DATA, offset2);                   // ICW2: Slave PIC vector offset

	Outb(PIC1_DATA, 4);                         // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	Outb(PIC2_DATA, 2);                         // ICW3: tell Slave PIC its cascade identity (0000 0010)
	
	Outb(PIC1_DATA, ICW4_8086);
	Outb(PIC2_DATA, ICW4_8086);
	
	Outb(PIC1_DATA, a1);                        // restore saved masks.
	Outb(PIC2_DATA, a2);
}

void PIC_SendEOI(uint8_t irqline) {
	if(irqline >= 8) Outb(PIC2_COMD, PIC_EOI);
 
	Outb(PIC1_COMD,PIC_EOI);
}

void PIC_EnableInterrupt(uint8_t irqline) {
	uint16_t port;
	uint8_t value;
	
	if(irqline < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		irqline -= 8;
	}
	value = Inb(port) | (1 << irqline);
	Outb(port, value);        
}
 
void PIC_DisableInterrupt(uint8_t irqline) {
	uint16_t port;
	uint8_t value;
	
	if(irqline < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		irqline -= 8;
	}
	value = Inb(port) & ~(1 << irqline);
	Outb(port, value);        
}

void PIC_SetInterruptMask(uint16_t mask) {
	uint8_t mask1 = (uint8_t)( mask & 0x00FF);
	uint8_t mask2 = (uint8_t)((mask & 0xFF00) >> 8);

	Outb(PIC1_DATA, mask1);
	Outb(PIC2_DATA, mask2);
}

void PIC_DisableAllInterrupts() {
	PIC_SetInterruptMask(0xFFFF);
}

uint16_t PIC_CombineInterruptMasks(uint8_t mask1, uint8_t mask2) {
	uint16_t mask  =  (uint16_t) mask1;
	         mask |= ((uint16_t) mask2) << 8;

	return mask;
}

uint16_t PIC_GetInterruptMaskRegister(void) {
    uint16_t wirr1 = (uint16_t) Inb(PIC1_DATA);
    uint16_t wirr2 = (uint16_t) Inb(PIC1_DATA) << 8;

    return wirr2 | wirr1;
}

uint16_t PIC_GetInterruptRequestRegister(void) {
    Outb(PIC1_COMD, PIC_READ_IRR); 
    Outb(PIC2_COMD, PIC_READ_IRR); 

	uint16_t wirr1 = (uint16_t) Inb(PIC1_COMD);
	uint16_t wirr2 = (uint16_t) Inb(PIC1_COMD) << 8;

    return wirr2 | wirr1;
}
 
uint16_t PIC_GetInServiceRegister(void) {
    Outb(PIC1_COMD, PIC_READ_ISR); 
    Outb(PIC2_COMD, PIC_READ_ISR); 

	uint16_t wisr1 = (uint16_t) Inb(PIC1_COMD);
	uint16_t wisr2 = (uint16_t) Inb(PIC1_COMD) << 8;

    return wisr2 | wisr1;
}
