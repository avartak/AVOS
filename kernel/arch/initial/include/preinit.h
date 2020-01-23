#ifndef KERNEL_PREINIT_H
#define KERNEL_PREINIT_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/core/setup/include/setup.h>
#include <kernel/arch/i386/include/paging.h>
#include <kernel/arch/i386/include/gdt.h>
#include <kernel/arch/i386/include/interrupts.h>

#define KERNEL_IDT_ADDENTRY(num) \
	extern void Interrupt_##num(); \
	X86_IDT_SetupEntry(&(Kernel_IDT[num]), (uintptr_t)Interrupt_##num, X86_GDT_SEG_KERN_CODE, X86_IDT_FLAGS_PRESENT | X86_IDT_FLAGS_DPL0 | X86_IDT_TYPE_INTR32);

extern uint32_t                  Kernel_pagedirectory[]__attribute__((aligned(X86_PAGING_PAGESIZE)));
extern struct X86_GDT_Entry      Kernel_GDT[];
extern struct X86_GDT_Descriptor Kernel_GDT_desc;
extern struct X86_TSS            Kernel_TSS;

extern struct X86_IDT_Entry      Kernel_IDT[0x100];
extern struct X86_IDT_Descriptor Kernel_IDT_desc;

extern void Initialize_Paging();
extern void Initialize_HigherHalfSwitch();
extern void Initialize_GDT();
extern void Initialize_IDT();
extern void Initialize_APICs();

#endif
