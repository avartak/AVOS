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
    struct TSS task_state;
    struct GDT_Entry gdt[GDT_NENTRIES];
    struct GDT_Descriptor gdt_desc;
    struct IDT_Entry idt[IDT_NENTRIES];
    struct IDT_Descriptor idt_desc;
}__attribute__((packed));

#endif
