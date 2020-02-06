#include <kernel/arch/i386/include/interrupt.h>
#include <kernel/arch/i386/include/idt.h>

void (*Interrupt_Handlers[X86_IDT_NENTRIES])(struct Interrupt_Frame*);

void Interrupt_BaseHandler(struct Interrupt_Frame* frame) {

	Interrupt_Handlers[frame->vector](frame);
}

void Interrupt_AddEntry(uint8_t entry, void (*handler)(struct Interrupt_Frame*)) {

	Interrupt_Handlers[entry] = handler;
}

void Interrupt_SetReturnRegister(struct Interrupt_Frame* frame, size_t value) {

	frame->eax = value;
}
