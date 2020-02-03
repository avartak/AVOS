#include <kernel/core/process/include/scheduler.h>
#include <kernel/core/process/include/state.h>
#include <kernel/core/process/include/process.h>
#include <kernel/core/process/include/context.h>
#include <kernel/core/synch/include/spinlock.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/arch/i386/include/gdt.h>
#include <kernel/arch/i386/include/controlregs.h>
#include <kernel/arch/initial/include/initialize.h>

struct Process Scheduler_processes[KERNEL_MAX_PROCS];

void Scheduler_RunProcess(struct Process* proc) {

	struct State* proc_state = (struct State*)(proc->kernel_thread + KERNEL_STACK_SIZE - sizeof(struct State));	
	proc_state->cpu = STATE_CURRENT->cpu;

	IRQLock_Acquire(&State_lock);

	STATE_CURRENT->process = proc;
    STATE_CURRENT->cpu->task_state.ss0 = X86_GDT_SEG_KERN_DATA;
    STATE_CURRENT->cpu->task_state.esp0 = (uintptr_t)proc_state;
    STATE_CURRENT->cpu->task_state.iomap_base_address = (uint16_t)0xFFFF;

	IRQLock_Release(&State_lock);

    X86_GDT_LoadTaskRegister(X86_GDT_SEG_USER_TSS | X86_GDT_RPL3);
    X86_CR3_Write((uintptr_t)proc->page_directory - KERNEL_HIGHER_HALF_OFFSET);

	proc->life_cycle = PROCESS_RUNNING;
	Context_Switch(&(STATE_CURRENT->cpu->scheduler), proc->context);

	X86_CR3_Write((uintptr_t)Kernel_pagedirectory);

}

void Scheduler_Return() {

	struct Process* proc = STATE_CURRENT->process;
	Context_Switch(&proc->context, STATE_CURRENT->cpu->scheduler);

}

void Schedule() {

	size_t iproc = 0;
	while (true) {
		SpinLock_Acquire(&Process_lock);

		if (Scheduler_processes[iproc].life_cycle == PROCESS_RUNNABLE) Scheduler_RunProcess(&Scheduler_processes[iproc]);
		iproc++;
		if (iproc == KERNEL_MAX_PROCS) iproc = 0;

		SpinLock_Release(&Process_lock);
	} 

}

void Scheduler_Initialize() {

	SpinLock_Initialize(&Process_lock, "process");
	for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) Scheduler_processes[i].life_cycle = PROCESS_UNUSED;

	IRQLock_Initialize(&State_lock, "state");
}
