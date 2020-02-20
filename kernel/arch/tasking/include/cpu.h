#ifndef KERNEL_X86_CPU_H
#define KERNEL_X86_CPU_H

#include <stdint.h>

#include <kernel/arch/processor/include/gdt.h>
#include <kernel/arch/processor/include/idt.h>
#include <kernel/arch/processor/include/functions.h>
#include <kernel/arch/tasking/include/interrupt.h>
#include <kernel/arch/tasking/include/context.h>

struct Process;

struct CPU {
    uint32_t apic_id;
    uint32_t acpi_id;
    struct X86_TSS task_state;
    struct X86_GDT_Entry gdt[X86_GDT_NENTRIES];
    struct X86_GDT_Descriptor gdt_desc;
    struct X86_IDT_Entry idt[X86_IDT_NENTRIES];
    struct X86_IDT_Descriptor idt_desc;
}__attribute__((packed));

#endif
