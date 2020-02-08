#include <kernel/arch/i386/include/cpu.h>
#include <kernel/arch/i386/include/controlregs.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/process/include/state.h>
#include <kernel/core/process/include/process.h>

void CPU_SetupProcess(struct Process* proc) {

    struct State* proc_state = (struct State*)(proc->kernel_thread + KERNEL_STACK_SIZE - sizeof(struct State));
    proc_state->cpu = STATE_CURRENT->cpu;

    STATE_CURRENT->cpu->task_state.ss0 = X86_GDT_SEG_KERN_DATA;
    STATE_CURRENT->cpu->task_state.esp0 = (uintptr_t)proc_state;
    STATE_CURRENT->cpu->task_state.iomap_base_address = (uint16_t)0xFFFF;

    X86_GDT_LoadTaskRegister(X86_GDT_SEG_USER_TSS | X86_GDT_RPL3);
    X86_CR3_Write((uintptr_t)proc->page_directory - KERNEL_HIGHER_HALF_OFFSET);
    proc->start_time = STATE_CURRENT->cpu->timer_ticks;
}
