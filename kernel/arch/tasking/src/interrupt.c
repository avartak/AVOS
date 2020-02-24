#include <kernel/core/setup/include/setup.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/taskmaster/include/process.h>
#include <kernel/arch/tasking/include/interrupt.h>
#include <kernel/arch/processor/include/gdt.h>
#include <kernel/arch/processor/include/idt.h>
#include <kernel/arch/apic/include/lapic.h>

void (*Interrupt_Handlers[X86_IDT_NENTRIES])(struct Interrupt_Frame*);

void Interrupt_Frame_Initialize(struct Process* proc) {

    uintptr_t stack_ptr = (uintptr_t)proc->kern_stack + KERNEL_STACK_SIZE;
   
    stack_ptr -= sizeof(struct State) + sizeof(struct Interrupt_Frame);
    proc->interrupt_frame = (struct Interrupt_Frame*)stack_ptr;

	stack_ptr -= sizeof(uintptr_t);
	*((uintptr_t*)stack_ptr) = (uintptr_t)Interrupt_Return;
}

void Interrupt_Frame_Fork(struct Interrupt_Frame* dst, struct Interrupt_Frame* src) {

    *dst = *src;
	dst->eax = 0;
}

void Interrupt_Handle(struct Interrupt_Frame* frame) {

	Interrupt_Handlers[frame->vector](frame);
	LocalAPIC_EOI();
	Process_Preempt();
}

void Interrupt_AddHandler(uint8_t entry, void (*handler)(struct Interrupt_Frame*)) {

	Interrupt_Handlers[entry] = handler;
}

size_t Interrupt_GetReturnRegister(struct Interrupt_Frame* frame) {
	
	return frame->eax;
}

extern size_t Interrupt_GetVector(struct Interrupt_Frame* frame) {

	return frame->vector;
}

bool Interrupt_ReturningToUserMode(struct Interrupt_Frame* frame) {

	return (frame->cs == X86_GDT_SEG_USER_CODE);
}

