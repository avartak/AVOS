#include <kernel/include/common.h>
#include <kernel/include/machine.h>
#include <kernel/include/multiboot.h>

extern void Paging_Initialize();
extern void Paging_SwitchToHigherHalf();
extern void GDT_Initialize();
extern void IDT_Initialize();
extern bool RAM_Initialize();
extern void Welcome();

extern void AVOS_Launch(uint32_t magic, uintptr_t bootinfo);
extern void AVOS_Halt();
extern void AVOS();

void AVOS_Launch(uint32_t magic, uintptr_t bootinfo) {

	if (magic != MULTIBOOT2_MAGIC) AVOS_Halt();

	Paging_Initialize();

	Paging_SwitchToHigherHalf();

	Multiboot_Info = bootinfo + KERNEL_HIGHER_HALF_OFFSET;

	GDT_Initialize();
	
	IDT_Initialize();

	if (!RAM_Initialize()) AVOS_Halt();

	Welcome();

	AVOS();

}

void AVOS_Halt() {

	while (true) {
		Interrupt_DisableAll();
		System_Halt();
	}		

}

