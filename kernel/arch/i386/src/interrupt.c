#include <kernel/core/process/include/state.h>
#include <kernel/core/process/include/process.h>
#include <kernel/core/process/include/scheduler.h>
#include <kernel/arch/i386/include/interrupt.h>
#include <kernel/arch/i386/include/gdt.h>
#include <kernel/arch/i386/include/idt.h>

void (*Interrupt_Handlers[X86_IDT_NENTRIES])(struct Interrupt_Frame*);

extern size_t Interrupt_Frame_GetStructSize() {

	return sizeof(struct Interrupt_Frame);
}

void Interrupt_BaseHandler(struct Interrupt_Frame* frame) {

	Interrupt_Handlers[frame->vector](frame);

	Scheduler_HandleInterruptReturn();
}

void Interrupt_AddEntry(uint8_t entry, void (*handler)(struct Interrupt_Frame*)) {

	Interrupt_Handlers[entry] = handler;
}

size_t Interrupt_GetReturnRegister(struct Interrupt_Frame* frame) {
	
	return frame->eax;
}

void Interrupt_SetReturnRegister(struct Interrupt_Frame* frame, size_t value) {

	frame->eax = value;
}

extern size_t Interrupt_GetVector(struct Interrupt_Frame* frame) {

	return frame->vector;
}

bool Interrupt_ReturningToUserMode(struct Interrupt_Frame* frame) {

	if (frame->cs == X86_GDT_SEG_USER_CODE) return true;
    else return false;
}

void Interrupt_CopyFrame(struct Interrupt_Frame* dst, struct Interrupt_Frame* src) {

	*dst = *src;
}
