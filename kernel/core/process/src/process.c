#include <kernel/core/process/include/process.h>
#include <kernel/core/process/include/state.h>
#include <kernel/core/process/include/scheduler.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/memory/include/physmem.h>
#include <kernel/core/memory/include/virtmem.h>
#include <kernel/clib/include/string.h>
#include <kernel/arch/i386/include/paging.h>
#include <kernel/arch/i386/include/controlregs.h>
#include <kernel/arch/i386/include/flags.h>
#include <kernel/arch/i386/include/functions.h>
#include <kernel/arch/initial/include/initialize.h>

struct SpinLock Process_lock;
size_t Process_next_pid = 1;

// The process lock will be held from Schedule()
void Process_FirstEntryToUserSpace() {

	SpinLock_Release(&Process_lock);

}

bool Process_Initialize(struct Process* proc) {

	if (proc->life_cycle != PROCESS_BOOKED) return false;

	// Allocate stack for the kernel thread
	void* kthread = Page_Acquire(KERNEL_STACK_SIZE >> KERNEL_PAGE_SIZE_IN_BITS);
	if (kthread == PAGE_LIST_NULL) return false;

	// Now we go about initializing the process
	proc->id = Process_next_pid++;
	proc->kernel_thread = (uint8_t*)kthread;

	uintptr_t stack_ptr = (uintptr_t)proc->kernel_thread + KERNEL_STACK_SIZE;
	
	// State goes at the very base of the stack 
	stack_ptr -= sizeof(struct State);
	((struct State*)stack_ptr)->process = proc;
	((struct State*)stack_ptr)->preemption_vetos = 0;
	((struct State*)stack_ptr)->interrupt_priority = 0;

	// Next comes the interrupt frame
	stack_ptr -= sizeof(struct Interrupt_Frame);
	proc->interrupt_frame = (struct Interrupt_Frame*)stack_ptr;
	
	// Then, there is space for the pointer to the interrupt frame (which gets passed to the interrupt handler)
	// Optionally it can also hold the address of the part of the interrupt handling code that handles return from the interrupt (to enter user space)
	stack_ptr -= sizeof(uintptr_t);
	*((uintptr_t*)stack_ptr) = (uintptr_t)Interrupt_Return;

	// The process context comes next	
	stack_ptr -= sizeof(struct Context);
	proc->context = (struct Context*)stack_ptr;
	memset(proc->context, 0, sizeof(struct Context));
	Context_SetProgramCounter(proc->context, (uintptr_t)Process_FirstEntryToUserSpace);

	return true;
}

// Must be run under process lock
void Process_Sleep() {

    struct Process* proc = STATE_CURRENT->process;
    if (proc == (struct Process*)0) return;
    if (STATE_CURRENT->preemption_vetos != 0) return;

    proc->wakeup_on = STATE_CURRENT->process;
    proc->life_cycle = PROCESS_ASLEEP;
    Scheduler_Return();
    proc->wakeup_on = (void*)0;

}

void Process_SleepOn(struct SleepLock* lock) {

    struct Process* proc = STATE_CURRENT->process;
    if (proc == (struct Process*)0) return;
    if (STATE_CURRENT->preemption_vetos != 0) return;

    SpinLock_Acquire(&Process_lock);
    SpinLock_Release(&lock->access_lock);

    proc->wakeup_on = lock;
    proc->life_cycle = PROCESS_ASLEEP;
    Scheduler_Return();
    proc->wakeup_on = (void*)0;

    SpinLock_Release(&Process_lock);
    SpinLock_Acquire(&lock->access_lock);
}

void Process_Yield() {

    struct Process* proc = STATE_CURRENT->process;
    if (proc == (struct Process*)0) return;

    SpinLock_Acquire(&Process_lock);

    if (proc->life_cycle != PROCESS_ZOMBIE && proc->life_cycle != PROCESS_KILLED) proc->life_cycle = PROCESS_RUNNABLE;
    Scheduler_Return();

    SpinLock_Release(&Process_lock);
}


uint32_t Process_ID() {

	struct Process* proc = STATE_CURRENT->process;
	if (proc == (struct Process*)0) return -1;

	return proc->id;
}

uint32_t Process_Fork() {

	SpinLock_Acquire(&Process_lock);
	struct Process* forked_proc = Scheduler_Book();
	SpinLock_Release(&Process_lock);

	if (forked_proc == (struct Process*)0) return -1;

	if (!Process_Initialize(forked_proc)) return -1;

	if (!Memory_MakePageDirectory(forked_proc->page_directory)) {
		Page_Release(forked_proc->kernel_thread, KERNEL_STACK_SIZE >> KERNEL_PAGE_SIZE_IN_BITS);

		SpinLock_Acquire(&Process_lock);
		forked_proc->life_cycle = PROCESS_IDLE;
		SpinLock_Release(&Process_lock);

		return -1;
	}

	forked_proc->parent = STATE_CURRENT->process;
	forked_proc->exit_status = -1;
	forked_proc->run_time = STATE_CURRENT->process->run_time;
	forked_proc->memory_endpoint = STATE_CURRENT->process->memory_endpoint;
	*(forked_proc->interrupt_frame) = *(STATE_CURRENT->process->interrupt_frame);
	Interrupt_SetReturnRegister(forked_proc->interrupt_frame, 0);

	SpinLock_Acquire(&Process_lock);
	forked_proc->life_cycle = PROCESS_RUNNABLE;
	SpinLock_Release(&Process_lock);

	return STATE_CURRENT->process->id;

}

void Process_ChangeMemoryEndPoint(int32_t shift) {

    struct Process* proc = STATE_CURRENT->process;
    if (proc == (struct Process*)0) return;

    uintptr_t old_end = proc->memory_endpoint;
    uintptr_t new_end = (int32_t)(proc->memory_endpoint) + shift;

    if (new_end == old_end) return;

    else if (new_end < old_end) {
        Memory_UnmapPages(proc->page_directory, (void*)new_end, old_end - new_end);
        proc->memory_endpoint = new_end;
    }

    else {
        uint16_t flags = X86_PAGING_PTE_PRESENT | X86_PAGING_PTE_READWRITE | X86_PAGING_PTE_USER;
        size_t size_alloc = Memory_MapPages(proc->page_directory, (void*)old_end, new_end - old_end, flags);
        uintptr_t old_page = old_end & ~(X86_PAGING_PAGESIZE-1);

        if (old_page + size_alloc < new_end) proc->memory_endpoint = old_page + size_alloc;
        else proc->memory_endpoint = new_end;
    }
}


// No locks should be held by the process when waiting
uint32_t Process_Wait() {

	SpinLock_Acquire(&Process_lock);

	uint32_t retval = -1;
	struct Process* proc = STATE_CURRENT->process;
	while (true) {
		if (!Scheduler_ProcessHasKids(proc) || proc->life_cycle == PROCESS_KILLED) {
			retval = -1;
			break;
		}
		struct Process* zombie_kid = Scheduler_GetProcessKid(proc, PROCESS_ZOMBIE);
		if (zombie_kid != (struct Process*)0) {
			retval = zombie_kid->id;
			zombie_kid->life_cycle = PROCESS_IDLE;
			break;
		}
		Process_Sleep();
	}
	
	SpinLock_Release(&Process_lock);

	return retval;
}

void Process_Exit(int status) {

    struct Process* proc = STATE_CURRENT->process;
    if (proc == (struct Process*)0) return;

    SpinLock_Acquire(&Process_lock);

	Scheduler_TerminateProcess(proc);
    proc->exit_status = status;

    Scheduler_Return();
}


