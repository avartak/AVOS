#include <x86/kernel/include/paging.h>
#include <x86/kernel/include/gdt.h>
#include <x86/kernel/include/idt.h>
#include <x86/kernel/include/physmem.h>
#include <x86/kernel/include/welcome.h>

extern uint32_t* MultibootInfo;
extern void      AVOS();

void LaunchAVOS(uint32_t* bootinfo) {

	if (! (Physical_Memory_CheckRange(0x100000, 0x400000, bootinfo) && Physical_Memory_CheckRange(0x1000000, 0x1400000, bootinfo)) ) return;

	Paging_Initialize();

	Paging_SwitchToHigherHalf();

	MultibootInfo = bootinfo + 0xC0000000/sizeof(uint32_t);

	GDT_Initialize();
	
	IDT_Initialize();

	Physical_Memory_Initialize();
	
	Welcome();

	AVOS();
}
