#include <x86/kernel/include/idt.h>
#include <x86/kernel/include/pic.h>

struct IDTEntry idt[256];
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

	for (uint16_t i = 0; i < 256; i++) SetupIDTEntry(&(idt[0] ), 0x00000000, 0x00, 0x00);

	uint8_t interrupt_type = (INT_ACCESS_KERNEL >> 4) | INT_TYPE_INTERRUPT;

    SetupIDTEntry(&(idt[0] ), (uintptr_t)Interrupt0 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[1] ), (uintptr_t)Interrupt1 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[2] ), (uintptr_t)Interrupt2 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[3] ), (uintptr_t)Interrupt3 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[4] ), (uintptr_t)Interrupt4 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[5] ), (uintptr_t)Interrupt5 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[6] ), (uintptr_t)Interrupt6 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[7] ), (uintptr_t)Interrupt7 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[8] ), (uintptr_t)Interrupt8 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[9] ), (uintptr_t)Interrupt9 , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[10]), (uintptr_t)InterruptA , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[11]), (uintptr_t)InterruptB , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[12]), (uintptr_t)InterruptC , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[13]), (uintptr_t)InterruptD , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[14]), (uintptr_t)InterruptE , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[15]), (uintptr_t)InterruptF , KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[16]), (uintptr_t)Interrupt10, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[17]), (uintptr_t)Interrupt11, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[18]), (uintptr_t)Interrupt12, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[19]), (uintptr_t)Interrupt13, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[20]), (uintptr_t)Interrupt14, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[21]), (uintptr_t)Interrupt15, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[22]), (uintptr_t)Interrupt16, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[23]), (uintptr_t)Interrupt17, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[24]), (uintptr_t)Interrupt18, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[25]), (uintptr_t)Interrupt19, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[26]), (uintptr_t)Interrupt1A, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[27]), (uintptr_t)Interrupt1B, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[28]), (uintptr_t)Interrupt1C, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[29]), (uintptr_t)Interrupt1D, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[30]), (uintptr_t)Interrupt1E, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[31]), (uintptr_t)Interrupt1F, KERNEL_CODE_SEG, interrupt_type);

    SetupIDTEntry(&(idt[32]), (uintptr_t)Interrupt20, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[33]), (uintptr_t)Interrupt21, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[34]), (uintptr_t)Interrupt22, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[35]), (uintptr_t)Interrupt23, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[36]), (uintptr_t)Interrupt24, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[37]), (uintptr_t)Interrupt25, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[38]), (uintptr_t)Interrupt26, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[39]), (uintptr_t)Interrupt27, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[40]), (uintptr_t)Interrupt28, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[41]), (uintptr_t)Interrupt29, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[42]), (uintptr_t)Interrupt2A, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[43]), (uintptr_t)Interrupt2B, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[44]), (uintptr_t)Interrupt2C, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[45]), (uintptr_t)Interrupt2D, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[46]), (uintptr_t)Interrupt2E, KERNEL_CODE_SEG, interrupt_type);
    SetupIDTEntry(&(idt[47]), (uintptr_t)Interrupt2F, KERNEL_CODE_SEG, interrupt_type);

    SetupIDTEntry(&(idt[80]), (uintptr_t)Interrupt80, KERNEL_CODE_SEG, interrupt_type);

    idtr.limit = (sizeof(struct IDTEntry))*256 - 1;
    idtr.base  = (uintptr_t)idt;
   
    LoadIDT(&idtr);

    return;

}


inline void LoadIDT(struct IDTRecord* idtr) {
    asm volatile ("lidt %0" : : "m"(*idtr));
}

void HandleInterrupt(struct CPUStateAtInterrupt cpu, struct StackStateAtInterrupt stack, uint32_t interrupt) {

	int syscallid = 0;

	if (stack.error_code != 0) {
	}

	if (interrupt >= 0x20 && interrupt < 0x30) {
		PIC_SendEOI(interrupt - 0x20);
	}

	if (interrupt == 0x80) {
		syscallid = cpu.eax;
	}

	return;

}

