#ifndef KERNEL_CORE_STATE_H
#define KERNEL_CORE_STATE_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

#include <kernel/arch/i386/include/gdt.h>
#include <kernel/arch/i386/include/interrupts.h>
#include <kernel/arch/i386/include/trap.h>
#include <kernel/arch/console/include/console.h>

struct CPU {
	uint32_t apic_id;
	uint32_t acpi_id;
	struct Context* scheduler;
	struct X86_TSS task_state;
	struct X86_GDT_Entry gdt[X86_GDT_NENTRIES];
	struct X86_GDT_Descriptor gdt_desc;
	struct X86_IDT_Entry idt[X86_IDT_NENTRIES];
	struct X86_IDT_Descriptor idt_desc;
}__attribute__((packed));

struct Context {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
}__attribute__((packed));

struct Process {
	uint32_t          id;
	struct Process*   parent;
	uint8_t           life_cycle;
	bool              killed;
	uint32_t*         page_directory;
	struct context*   context;
	uint8_t*          kernel_thread;
	struct TrapFrame* trap_frame;
	void*             wakeup_on;
}__attribute__((packed));

struct State {
	size_t          preemption_vetos;
	uint8_t         interrupt_priority;
	struct Process* process;
	struct CPU*     cpu;
}__attribute__((packed));

extern struct State* State_GetCurrent();
extern struct CPU*   State_GetCPU();

#define STATE_INCREMENT_PREEMPTION_VETO() \
	do { \
		State_GetCurrent()->preemption_vetos++; \
		atomic_signal_fence(memory_order_seq_cst); \
	} while (0)

#define STATE_DECREMENT_PREEMPTION_VETO() \
    do { \
		if (State_GetCurrent()->preemption_vetos == 0) Console_Panic("Panic(STATE_DECREMENT_PREEMPTION_VETO): No existing veto on preemption\n"); \
        atomic_signal_fence(memory_order_seq_cst); \
        State_GetCurrent()->preemption_vetos--; \
    } while (0)

#endif
