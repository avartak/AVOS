#include <kernel/core/process/include/state.h>
#include <kernel/core/process/include/process.h>
#include <kernel/core/process/include/scheduler.h>
#include <kernel/arch/i386/include/interrupt.h>
#include <kernel/arch/i386/include/idt.h>

void (*Interrupt_Handlers[X86_IDT_NENTRIES])(struct Interrupt_Frame*);

void Interrupt_BaseHandler(struct Interrupt_Frame* frame) {

	Interrupt_Handlers[frame->vector](frame);

	Scheduler_HandleInterruptReturn();
}

void Interrupt_AddEntry(uint8_t entry, void (*handler)(struct Interrupt_Frame*)) {

	Interrupt_Handlers[entry] = handler;
}

void Interrupt_SetReturnRegister(struct Interrupt_Frame* frame, size_t value) {

	frame->eax = value;
}

bool Interrupt_ReturningToUserMode(struct Interrupt_Frame* frame) {

	if (frame->cs == X86_GDT_SEG_USER_CODE) return true;
    else return false;
}
