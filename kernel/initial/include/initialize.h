#ifndef KERNEL_INITIALIZE_H
#define KERNEL_INITIALIZE_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/initial/include/setup.h>
#include <kernel/arch/include/paging.h>
#include <kernel/arch/include/gdt.h>
#include <kernel/arch/include/interrupts.h>

extern uint32_t                  Kernel_pagedirectory[]__attribute__((aligned(X86_PAGING_PAGESIZE)));
extern struct X86_GDT_Entry      Kernel_GDT[];
extern struct X86_GDT_Descriptor Kernel_GDT_desc;
extern struct X86_TSS            Kernel_TSS;

extern void Initialize_Paging();
extern void Initialize_SwitchToHigherHalf();
extern void Initialize_GDT();
extern void Initialize_IDT();

#endif
