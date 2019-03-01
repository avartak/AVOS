#include <x86/kernel/include/paging.h>
#include <x86/kernel/include/gdt.h>
#include <x86/kernel/include/idt.h>
#include <x86/kernel/include/physmem.h>
#include <x86/kernel/include/welcome.h>
#include <x86/drivers/include/pic.h>
#include <x86/drivers/include/pit.h>
#include <kernel/include/machine.h>

extern void Kmain();

void Kinit() {

	Paging_Initialize();

	Paging_SwitchToHigherHalf();

	GDT_Initialize();
	
	IDT_Initialize();
	
	Physical_Memory_Initialize();
	
	Welcome();

	Kmain();

}
