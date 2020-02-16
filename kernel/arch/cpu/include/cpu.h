#ifndef KERNEL_X86_CPU_H
#define KERNEL_X86_CPU_H

#include <stdint.h>

#include <kernel/arch/i386/include/gdt.h>
#include <kernel/arch/i386/include/idt.h>
#include <kernel/arch/i386/include/interrupt.h>
#include <kernel/arch/i386/include/functions.h>
#include <kernel/arch/cpu/include/context.h>

struct Process;

struct CPU {
    uint32_t apic_id;
    uint32_t acpi_id;
    struct Context* scheduler;
    struct X86_TSS task_state;
    struct X86_GDT_Entry gdt[X86_GDT_NENTRIES];
    struct X86_GDT_Descriptor gdt_desc;
    struct X86_IDT_Entry idt[X86_IDT_NENTRIES];
    struct X86_IDT_Descriptor idt_desc;
    uint64_t timer_ticks;
}__attribute__((packed));

extern void     CPU_SetupProcess(struct Process* proc);
extern void     CPU_CleanupProcess(struct Process* proc);
extern size_t   CPU_GetStructSize();
extern uint64_t CPU_GetTimerTicks(struct CPU* cpu);
extern struct Context* CPU_GetScheduler(struct CPU* cpu);
extern struct Context** CPU_GetSchedulerPtr(struct CPU* cpu);

#endif
