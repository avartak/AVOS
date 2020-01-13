#include <kernel/x86/kernel/include/paging.h>

uint32_t Paging_directory[0x400]__attribute__((aligned(0x1000)));
uint32_t Paging_kerntable[0x400]__attribute__((aligned(0x1000)));

void Paging_Initialize() {
	uintptr_t ipd = (uintptr_t)Paging_directory - KERNEL_HIGHER_HALF_OFFSET;
	uintptr_t ipt = (uintptr_t)Paging_kerntable - KERNEL_HIGHER_HALF_OFFSET;
	uint32_t*  pd = (uint32_t*)ipd;
	uint32_t*  pt = (uint32_t*)ipt;

	for (size_t i = 0; i < 0x400; i++) {
		pd[i] = 0;	
		pt[i] = i * 0x1000 + PAGING_KERN_PAGE_FLAGS;
	}

	Paging_MapEntry(pd, ipt,     0, PAGING_KERN_PAGE_FLAGS);
	Paging_MapEntry(pd, ipt, 0x300, PAGING_KERN_PAGE_FLAGS);
	Paging_MapEntry(pd, ipd, 0x3FF, PAGING_KERN_PAGE_FLAGS);

	Paging_LoadDirectory(ipd);
	Paging_Enable();
}
