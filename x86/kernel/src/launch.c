#include <x86/kernel/include/paging.h>
#include <x86/kernel/include/gdt.h>
#include <x86/kernel/include/idt.h>
#include <x86/kernel/include/ram.h>
#include <x86/kernel/include/physmem.h>
#include <x86/kernel/include/welcome.h>

extern uintptr_t Multiboot_Info;
extern void      AVOS();

void LaunchAVOS(uintptr_t bootinfo) {

	Paging_Initialize();

	Paging_SwitchToHigherHalf();

	Multiboot_Info = bootinfo + KERNEL_HIGHER_HALF_OFFSET;

	GDT_Initialize();
	
	IDT_Initialize();

	RAM_Initialize();

	if (! (RAM_IsMemoryPresent(0x100000, 0x400000) && RAM_IsMemoryPresent(0x1000000, 0x1400000)) ) return;

	Physical_Memory_Initialize();
	
	Welcome();

	AVOS();

}
