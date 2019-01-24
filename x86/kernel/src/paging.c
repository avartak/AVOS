#include <x86/kernel/include/paging.h>

#include <stdint.h>
#include <stddef.h>

uint32_t Paging_kernel_directory[0x400]__attribute__((aligned(0x1000)));
uint32_t Paging_kernel_selftable[0x400]__attribute__((aligned(0x1000)));

void Paging_MapTableInDirectory(uintptr_t pd, uintptr_t pt, uint32_t entry, uint16_t attr) {

	entry &= 0x03FF; 
	attr  &= 0x0FFF; 
	pt    &= 0xFFFFF000;
	pt    |= attr;

	((uint32_t*)pd)[entry] = pt;

}

void Paging_MapPageInTable(uintptr_t pt, uintptr_t pg, uint32_t entry, uint16_t attr) {

	entry &= 0x03FF; 
	attr  &= 0x0FFF; 
	pg    &= 0xFFFFF000;
	pg    |= attr;

	((uint32_t*)pt)[entry] = pg;

}

void Paging_MapMemoryBlockInTable(uintptr_t pt, uintptr_t addr, uint16_t attr) {

	uint32_t* table = (uint32_t*)pt;

	attr &= 0x0FFF; 
	addr &= 0xFFFFF000;
	addr |= attr;

    for(size_t i = 0; i < 1024; i++) {
        table[i] = i * 0x1000 + addr;
    }
	

}

void Paging_Initialize() {

	uintptr_t pd = (uintptr_t)Paging_kernel_directory;
	uintptr_t pt = (uintptr_t)Paging_kernel_selftable;

	pd = (pd > 0xC0000000 ? pd - 0xC0000000 : pd);
	pt = (pt > 0xC0000000 ? pt - 0xC0000000 : pt);

	for (size_t i = 0; i < 1024; i++) Paging_kernel_directory[i] = 0;	

	Paging_MapMemoryBlockInTable(pt, 0, 3);

	Paging_MapTableInDirectory  (pd, pt,   0, 3);
	Paging_MapTableInDirectory  (pd, pt, 768, 3);

	Paging_LoadDirectory(pd > 0xC0000000 ? pd - 0xC0000000 : pd);
	Paging_EnablePGBitInCR0();

	asm volatile("add $0xC0000000, %esp");

	Paging_kernel_directory[0] = 0;

}
