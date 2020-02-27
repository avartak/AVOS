#include <kernel/arch/tasking/include/context.h>
#include <kernel/arch/tasking/include/interrupt.h>
#include <kernel/arch/tasking/include/cpu.h>
#include <kernel/arch/processor/include/gdt.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/taskmaster/include/process.h>
#include <kernel/clib/include/string.h>

void Context_Initialize(struct Process* proc) {

    uintptr_t stack_ptr = (uintptr_t)STATE_FROM_PROC(proc);

    stack_ptr -= sizeof(struct IContext);

    proc->intr_context = (struct IContext*)stack_ptr;

    stack_ptr -= sizeof(uintptr_t);
    *((uintptr_t*)stack_ptr) = (uintptr_t)Interrupt_Return;
	
    stack_ptr -= sizeof(struct KContext);

    proc->task_context = (struct KContext*)stack_ptr;
    memset(proc->task_context, 0, sizeof(struct KContext));
    proc->task_context->eip = (uintptr_t)Process_FirstEntryToUserSpace;
}

void Context_SetupProcess(struct Process* proc) {

    STATE_FROM_PROC(proc)->kernel_task = STATE_CURRENT->kernel_task;
    STATE_FROM_PROC(proc)->kernel_task->cpu->task_state.esp0 = (uintptr_t)STATE_FROM_PROC(proc);
    proc->start_time = STATE_CURRENT->kernel_task->timer_ticks;
}


bool IContext_IsUserMode(struct IContext* frame) {

	return (frame->cs == X86_GDT_SEG_USER_CODE);
}

void IContext_Copy(struct IContext* dst, struct IContext* src) {

    *dst = *src;
}

void IContext_Fork(struct IContext* child, struct IContext* parent) {

    *child = *parent;
    child->eax = 0;
}

