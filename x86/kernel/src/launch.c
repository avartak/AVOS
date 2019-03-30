#include <kernel/include/common.h>

extern uintptr_t Multiboot_Info;

extern void Paging_Initialize();
extern void Paging_SwitchToHigherHalf();
extern void GDT_Initialize();
extern void IDT_Initialize();
extern bool RAM_Initialize();
extern void Memory_Physical_Initialize();
extern void Welcome();
extern void AVOS();

void LaunchAVOS(uintptr_t bootinfo) {

	Paging_Initialize();

	Paging_SwitchToHigherHalf();

	Multiboot_Info = bootinfo + KERNEL_HIGHER_HALF_OFFSET;

	GDT_Initialize();
	
	IDT_Initialize();

	if (!RAM_Initialize()) return;

	Welcome();

	AVOS();

}
