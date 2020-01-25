#include <kernel/arch/apic/include/pic.h>
#include <kernel/arch/i386/include/ioports.h>

bool    PIC_exists;
uint8_t PIC_InitialMask1;
uint8_t PIC_InitialMask2;

void PIC_Initialize() {

	PIC_InitialMask1 = X86_Inb(PIC_IOPORT_DATA1);                       // Save the state of masks at the point of PIC initialization
	PIC_InitialMask2 = X86_Inb(PIC_IOPORT_DATA2); 

	PIC_DisableAllInterrupts();

	X86_Outb(PIC_IOPORT_COMD1, PIC_ICW1_INIT | PIC_ICW1_NEED_ICW4);     // starts the initialization sequence (in cascade mode)
	X86_Outb(PIC_IOPORT_COMD2, PIC_ICW1_INIT | PIC_ICW1_NEED_ICW4);     // ICW1_ICW4 bit says that there will be a ICW4 command word coming

	X86_Outb(PIC_IOPORT_DATA1, PIC_REMAP1_START);                       // ICW2: Master PIC vector offset
	X86_Outb(PIC_IOPORT_DATA2, PIC_REMAP2_START);                       // ICW2: Slave  PIC vector offset

	X86_Outb(PIC_IOPORT_DATA1, 4);                                      // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	X86_Outb(PIC_IOPORT_DATA2, 2);                                      // ICW3: tell Slave PIC its cascade identity (0000 0010)
	
	X86_Outb(PIC_IOPORT_DATA1, PIC_ICW4_8086);
	X86_Outb(PIC_IOPORT_DATA2, PIC_ICW4_8086);
	
	PIC_DisableAllInterrupts();
	
}

void PIC_SendEOI(uint8_t irqline) {
	if (irqline >= 8) X86_Outb(PIC_IOPORT_COMD2, PIC_EOI);
 
	X86_Outb(PIC_IOPORT_COMD1, PIC_EOI);
}

void PIC_EnableInterrupt(uint8_t irqline) {
	uint16_t port;
	uint8_t value;
	
	if(irqline < 8) {
		port = PIC_IOPORT_DATA1;
	} else {
		port = PIC_IOPORT_DATA2;
		irqline -= 8;
	}
	value = X86_Inb(port) & ~(1 << irqline);
	X86_Outb(port, value);        
}

void PIC_DisableInterrupt(uint8_t irqline) {
	uint16_t port;
	uint8_t value;
	
	if(irqline < 8) {
		port = PIC_IOPORT_DATA1;
	} else {
		port = PIC_IOPORT_DATA2;
		irqline -= 8;
	}
	value = X86_Inb(port) | (1 << irqline);
	X86_Outb(port, value);        
}
 
void PIC_SetInterruptMask(uint16_t mask) {
	uint8_t mask1 = (uint8_t)( mask & 0x00FF);
	uint8_t mask2 = (uint8_t)((mask & 0xFF00) >> 8);

	X86_Outb(PIC_IOPORT_DATA1, mask1);
	X86_Outb(PIC_IOPORT_DATA2, mask2);
}

void PIC_DisableAllInterrupts() {
	X86_Outb(PIC_IOPORT_DATA1, 0xFF);
	X86_Outb(PIC_IOPORT_DATA2, 0xFF);
}

uint16_t PIC_CombineInterruptMasks(uint8_t mask1, uint8_t mask2) {
	uint16_t mask  =  (uint16_t) mask1;
	         mask |= ((uint16_t) mask2) << 8;

	return mask;
}

void PIC1_SetInterruptMask(uint8_t mask) {
    X86_Outb(PIC_IOPORT_DATA1, mask);
}

void PIC1_DisableAllInterrupts() {
	X86_Outb(PIC_IOPORT_DATA1, 0xFF);
}

void PIC2_SetInterruptMask(uint8_t mask) {
    X86_Outb(PIC_IOPORT_DATA2, mask);
}

void PIC2_DisableAllInterrupts() {
	X86_Outb(PIC_IOPORT_DATA2, 0xFF);
}

uint16_t PIC_GetInterruptMaskRegister(void) {
    uint16_t wirr1 = (uint16_t) X86_Inb(PIC_IOPORT_DATA1);
    uint16_t wirr2 = (uint16_t) X86_Inb(PIC_IOPORT_DATA1) << 8;

    return wirr2 | wirr1;
}

uint16_t PIC_GetInterruptRequestRegister(void) {
    X86_Outb(PIC_IOPORT_COMD1, PIC_READ_IRR); 
    X86_Outb(PIC_IOPORT_COMD2, PIC_READ_IRR); 

	uint16_t wirr1 = (uint16_t) X86_Inb(PIC_IOPORT_COMD1);
	uint16_t wirr2 = (uint16_t) X86_Inb(PIC_IOPORT_COMD1) << 8;

    return wirr2 | wirr1;
}
 
uint16_t PIC_GetInServiceRegister(void) {
    X86_Outb(PIC_IOPORT_COMD1, PIC_READ_ISR); 
    X86_Outb(PIC_IOPORT_COMD2, PIC_READ_ISR); 

	uint16_t wisr1 = (uint16_t) X86_Inb(PIC_IOPORT_COMD1);
	uint16_t wisr2 = (uint16_t) X86_Inb(PIC_IOPORT_COMD1) << 8;

    return wisr2 | wisr1;
}

uint8_t PIC1_GetInterruptMaskRegister(void) {
    uint8_t irr = X86_Inb(PIC_IOPORT_DATA1);

    return  irr;
}

uint8_t PIC1_GetInterruptRequestRegister(void) {
    X86_Outb(PIC_IOPORT_COMD1, PIC_READ_IRR);

    uint8_t irr = X86_Inb(PIC_IOPORT_COMD1);

    return  irr;
}

uint8_t PIC1_GetInServiceRegister(void) {
    X86_Outb(PIC_IOPORT_COMD1, PIC_READ_ISR);

    uint8_t isr = X86_Inb(PIC_IOPORT_COMD1);

    return  isr;
}

uint8_t PIC2_GetInterruptMaskRegister(void) {
    uint8_t irr = X86_Inb(PIC_IOPORT_DATA2);
    
    return  irr;
}

uint8_t PIC2_GetInterruptRequestRegister(void) {
    X86_Outb(PIC_IOPORT_COMD2, PIC_READ_IRR);

    uint8_t irr = X86_Inb(PIC_IOPORT_COMD2);

    return  irr;
}

uint8_t PIC2_GetInServiceRegister(void) {
    X86_Outb(PIC_IOPORT_COMD2, PIC_READ_ISR);

    uint8_t isr = X86_Inb(PIC_IOPORT_COMD2);

    return  isr;
}
