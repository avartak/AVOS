#include <kernel/core/arch/include/arch.h>
#include <kernel/core/taskmaster/include/process.h>
#include <kernel/core/taskmaster/include/scheduler.h>
#include <kernel/core/taskmaster/include/dispatcher.h>
#include <kernel/core/memory/include/physmem.h>

// Must run under the process lock
void Dispatch(struct Process* proc) {

	proc->life_cycle = PROCESS_RUNNING;
	Context_SetupProcess(proc);
	SCHEDULER_SWITCH(proc);
}

// Must run under the process lock
void Dispatcher_TerminateProcess(struct Process* proc, int exit_status) {

    struct Process* kid = proc->child;
    while (kid != PROCESS_NULL) {
        kid->parent = Process_primordial;
        if (kid->life_cycle == PROCESS_DEAD && Process_primordial->life_cycle == PROCESS_WAITING) Process_primordial->life_cycle = PROCESS_RUNNABLE;
        kid = kid->sibling;
    }

    Paging_Terminate(proc);
    Page_Release(proc->kstack, KERNEL_STACK_SIZE >> KERNEL_PAGE_SIZE_IN_BITS);
    proc->exit_status = exit_status;
    proc->life_cycle = PROCESS_DEAD;

    if (proc->parent->life_cycle == PROCESS_WAITING) proc->parent->life_cycle = PROCESS_RUNNABLE;
}

