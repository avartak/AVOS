#include <kernel/core/process/include/state.h>
#include <kernel/core/process/include/process.h>
#include <kernel/core/process/include/scheduler.h>
#include <kernel/arch/i386/include/interrupt.h>
#include <kernel/arch/i386/include/idt.h>

void (*Interrupt_Handlers[X86_IDT_NENTRIES])(struct Interrupt_Frame*);

void Interrupt_BaseHandler(struct Interrupt_Frame* frame) {

	struct Process* proc = STATE_CURRENT->process;
	if (frame->vector == 0x80) {
		if (proc != (struct Process*)0 && proc->life_cycle == PROCESS_KILLED) Process_Exit(proc->exit_status);
	}

	Interrupt_Handlers[frame->vector](frame);

	if (proc != (struct Process*)0 && proc->life_cycle == PROCESS_KILLED && proc->interrupt_frame->cs == X86_GDT_SEG_USER_CODE) Process_Exit(proc->exit_status);
	if (proc != (struct Process*)0 && proc->life_cycle == PROCESS_RUNNING && Scheduler_ProcessShouldYield(proc)) Process_Yield();
	if (proc != (struct Process*)0 && proc->life_cycle == PROCESS_KILLED && proc->interrupt_frame->cs == X86_GDT_SEG_USER_CODE) Process_Exit(proc->exit_status);
}

void Interrupt_AddEntry(uint8_t entry, void (*handler)(struct Interrupt_Frame*)) {

	Interrupt_Handlers[entry] = handler;
}

void Interrupt_SetReturnRegister(struct Interrupt_Frame* frame, size_t value) {

	frame->eax = value;
}
