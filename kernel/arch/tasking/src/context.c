#include <kernel/arch/tasking/include/context.h>
#include <kernel/arch/tasking/include/interrupt.h>
#include <kernel/arch/tasking/include/cpu.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/taskmaster/include/process.h>
#include <kernel/clib/include/string.h>

void Context_Initialize(struct Process* proc) {

    uintptr_t stack_ptr = (uintptr_t)proc->kstack + KERNEL_STACK_SIZE;
    stack_ptr -= sizeof(struct State) + sizeof(struct Interrupt_Frame) + sizeof(uintptr_t) + sizeof(struct Context);

    proc->context = (struct Context*)stack_ptr;
    memset(proc->context, 0, sizeof(struct Context));
    proc->context->eip = (uintptr_t)Process_FirstEntryToUserSpace;
}

void Context_SetupProcess(struct Process* proc) {

    struct State* proc_state = (struct State*)(proc->kstack + KERNEL_STACK_SIZE - sizeof(struct State));
    proc_state->kernel_task = STATE_CURRENT->kernel_task;

    STATE_CURRENT->kernel_task->cpu->task_state.esp0 = (uintptr_t)proc_state;
    proc->start_time = STATE_CURRENT->kernel_task->timer_ticks;
}

