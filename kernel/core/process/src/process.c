#include <kernel/core/process/include/process.h>
#include <kernel/core/process/include/state.h>
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
struct Process_List* Process_infocus = (struct Process_List*)0xFFFFFFFF;
size_t Process_count = 0;

void Process_ChangeMemoryEndPoint(struct Process* proc, int32_t shift) {

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

void Process_PrepareSwitch(struct Process* proc) {

	IRQLock_Acquire(&State_lock);

    STATE_CURRENT->cpu->task_state.ss0 = X86_GDT_SEG_KERN_DATA;
    STATE_CURRENT->cpu->task_state.esp0 = (uintptr_t)proc->kernel_thread + KERNEL_STACK_SIZE - sizeof(struct State);
    STATE_CURRENT->cpu->task_state.iomap_base_address = (uint16_t)0xFFFF;
    X86_GDT_LoadTaskRegister(X86_GDT_SEG_USER_TSS | X86_GDT_RPL3);
    X86_CR3_Write((uintptr_t)proc->page_directory - KERNEL_HIGHER_HALF_OFFSET);

	IRQLock_Release(&State_lock);
}

bool Process_Setup(struct Process* proc) {

	if (proc->life_cycle != PROCESS_EMBRYO) return false;

	// Allocate stack for the kernel thread
	void* kthread = Page_Acquire(0);
	if (kthread == PAGE_LIST_NULL) return false;
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
	proc->context->eip = (uintptr_t)Process_FirstEntryToUserSpace;

	return true;
}

void Process_FirstEntryToUserSpace() {

	SpinLock_Release(&Process_lock);

}	

bool Process_Add() {

	struct Process_List* proc_new;

	if (Process_infocus == (struct Process_List*)0xFFFFFFFF) {
		void* page = Page_Acquire(0);
		if (page == PAGE_LIST_NULL) return false;
		proc_new = (struct Process_List*)page;
	}

	else if (((uintptr_t)Process_infocus & ~(X86_PAGING_PAGESIZE-1)) != ((uintptr_t)(Process_infocus+1) & ~(X86_PAGING_PAGESIZE-1))) {
		void* page = Page_Acquire(0);
		if (page == PAGE_LIST_NULL) return false;
		proc_new = (struct Process_List*)page;
	}

	else {
		proc_new = Process_infocus+1;
	}

	if (Process_infocus == (struct Process_List*)0xFFFFFFFF) {
		Process_infocus = proc_new;
		Process_infocus->next = Process_infocus;
		Process_infocus->prev = Process_infocus;
	}
	else {
		proc_new->next = Process_infocus->next;
		proc_new->prev = Process_infocus;
		Process_infocus->next = proc_new;
		Process_infocus = proc_new;
	}

	(Process_infocus->process).life_cycle = PROCESS_UNUSED;
	Process_count++;

	return true;
}

void Process_Initialize() {

	SpinLock_Initialize(&Process_lock, "process");
	for (size_t i = 0; i < KERNEL_NPROCS_INITIAL; i++) Process_Add();

	IRQLock_Initialize(&State_lock, "state");
}
