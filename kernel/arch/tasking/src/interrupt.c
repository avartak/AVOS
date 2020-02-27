#include <kernel/core/setup/include/setup.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/taskmaster/include/process.h>
#include <kernel/core/syscall/include/syscall.h>
#include <kernel/arch/tasking/include/interrupt.h>
#include <kernel/arch/tasking/include/context.h>
#include <kernel/arch/processor/include/gdt.h>
#include <kernel/arch/processor/include/idt.h>
#include <kernel/arch/apic/include/lapic.h>

void (*Interrupt_Handlers[X86_IDT_NENTRIES])(struct IContext*);

void Interrupt_Handle(struct IContext* frame) {

	if (frame->vector == 0x80) SysCall(frame->eax, frame); 
	Interrupt_Handlers[frame->vector](frame);
}

void Interrupt_AddHandler(uint8_t entry, void (*handler)(struct IContext*)) {

	Interrupt_Handlers[entry] = handler;
}

