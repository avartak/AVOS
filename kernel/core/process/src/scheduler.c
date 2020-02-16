#include <kernel/core/process/include/scheduler.h>
#include <kernel/core/process/include/state.h>
#include <kernel/core/process/include/process.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/memory/include/physmem.h>
#include <kernel/core/syscall/include/syscall.h>

struct Process Scheduler_processes[KERNEL_MAX_PROCS];

// Initialized the processes and related locks 
void Scheduler_Initialize() {

	SpinLock_Initialize(&Process_lock);
	for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) Scheduler_processes[i].life_cycle = PROCESS_IDLE;
	SpinLock_Initialize(&State_lock);

	SysCall_Initialize(0x80);
}

void Schedule() {

	size_t iproc = 0;
	while (true) {
		SpinLock_Acquire(&Process_lock);

		if (Scheduler_processes[iproc].life_cycle == PROCESS_ASLEEP && Scheduler_processes[iproc].signaled_change == PROCESS_RUNNABLE) {
			Scheduler_processes[iproc].signaled_change = PROCESS_UNDEFINED;
			Scheduler_processes[iproc].life_cycle = PROCESS_RUNNABLE;
		}
		if (Scheduler_processes[iproc].life_cycle == PROCESS_RUNNABLE) {
			CPU_SetupProcess(&Scheduler_processes[iproc]);
			Scheduler_processes[iproc].life_cycle = PROCESS_RUNNING;
			SCHEDULER_SWITCH(&Scheduler_processes[iproc]);
			CPU_CleanupProcess(&Scheduler_processes[iproc]);
		}
		iproc = (iproc == KERNEL_MAX_PROCS-1) ? 0 : iproc + 1;

		SpinLock_Release(&Process_lock);
	} 

}

void Scheduler_HandleInterruptReturn() {

	struct Process* proc = STATE_CURRENT->process;	

    if (proc == (struct Process*)0) return;

    if (STATE_CURRENT->preemption_vetos != 0) return;

	SpinLock_Acquire(&Process_lock);

	if (Interrupt_ReturningToUserMode(proc->interrupt_frame)) {
		if (proc->signaled_change == PROCESS_KILLED) Scheduler_TerminateProcess(proc);
		if (proc->signaled_change == PROCESS_ASLEEP) {
			proc->signaled_change = PROCESS_UNDEFINED;
			Process_Sleep();
		}
	}

    if (proc->life_cycle == PROCESS_RUNNING && Interrupt_GetVector(proc->interrupt_frame) == 0x30) {
		if (CPU_GetTimerTicks(STATE_CURRENT->cpu) > proc->start_time + proc->run_time) {
			proc->life_cycle = PROCESS_RUNNABLE;
			SCHEDULER_RETURN;
		}
	}

	if (Interrupt_ReturningToUserMode(proc->interrupt_frame)) {
		if (proc->signaled_change == PROCESS_KILLED) Scheduler_TerminateProcess(proc);
		if (proc->signaled_change == PROCESS_ASLEEP) {
			proc->signaled_change = PROCESS_UNDEFINED;
			Process_Sleep();
		}
	}

	SpinLock_Release(&Process_lock);
}

// Must run under the process lock
struct Process* Scheduler_Book() {

	for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
		if (Scheduler_processes[i].life_cycle == PROCESS_IDLE) {
			Scheduler_processes[i].life_cycle = PROCESS_BOOKED;
			Scheduler_processes[i].id = Process_next_pid++;
			return &Scheduler_processes[i];
		}
	}
	
	return (struct Process*)0;
}

// Must run under the process lock
void Scheduler_ChangeParent(struct Process* old_parent, struct Process* new_parent) {

    for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
        if (Scheduler_processes[i].parent == old_parent) {
			Scheduler_processes[i].parent = new_parent;
            if (Scheduler_processes[i].life_cycle == PROCESS_ZOMBIE) Scheduler_Wakeup(new_parent);
        }
    }

}

// Must run under the process lock
void Scheduler_Wakeup(void* alarm) {

    for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
        if (Scheduler_processes[i].life_cycle == PROCESS_ASLEEP && Scheduler_processes[i].wakeup_on == alarm) {
			Scheduler_processes[i].life_cycle = PROCESS_RUNNABLE;
			Scheduler_processes[i].wakeup_on  = (void*)0;
		}
		
    }

}

// Must run under the process lock
struct Process* Scheduler_GetProcessKid(struct Process* proc, enum Process_LifeCycle cycle) {

	struct Process* kid = (struct Process*)(-1);
    for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
        if (Scheduler_processes[i].parent == proc) {
			kid = (struct Process*)0;
			if (Scheduler_processes[i].life_cycle == cycle) return &Scheduler_processes[i];
		}
    }
	return kid;

}

// Must run under the process lock
void Scheduler_LifeCycleChangeSignal(uint32_t process_id, enum Process_LifeCycle requested_cycle) {

	if (requested_cycle != PROCESS_KILLED && requested_cycle != PROCESS_ASLEEP && requested_cycle != PROCESS_RUNNABLE) return;

    for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
        if (Scheduler_processes[i].id == process_id && Scheduler_processes[i].life_cycle != PROCESS_IDLE && Scheduler_processes[i].life_cycle != PROCESS_ZOMBIE) {
            Scheduler_processes[i].signaled_change = requested_cycle;
        }
    }
}

// Must run under the process lock
void Scheduler_TerminateProcess(struct Process* proc) {

    Scheduler_ChangeParent(proc, (struct Process*)0);
    proc->life_cycle = PROCESS_ZOMBIE;
    Paging_UnmakePageDirectory(proc->page_directory);
    Page_Release(proc->kernel_thread, KERNEL_STACK_SIZE >> KERNEL_PAGE_SIZE_IN_BITS);
    Scheduler_Wakeup(proc->parent);
}
