#include <x86/kernel/include/paging.h>

#include <stdint.h>
#include <stddef.h>

uint32_t Paging_directory[0x400]__attribute__((aligned(0x1000)));

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

	uintptr_t pd = (uintptr_t)Paging_directory;

	for (size_t i = 0; i < 1024; i++) Paging_directory[i] = 0;	

	uintptr_t ktable = LOC_PAGEMAP_PM;
	uintptr_t ptable = LOC_PAGEMAP_PM + 0x1000;

	Paging_MapMemoryBlockInTable(ktable,              0, 3);
	Paging_MapMemoryBlockInTable(ptable, LOC_PAGEMAP_PM, 3);

	Paging_MapTableInDirectory(pd, ktable,   0, 3);
	Paging_MapTableInDirectory(pd, ptable,   1, 3);
	Paging_MapTableInDirectory(pd, ktable, 768, 3);
	Paging_MapTableInDirectory(pd, ptable, 769, 3);

	Paging_LoadDirectory(pd > 0xC0000000 ? pd - 0xC0000000 : pd);
	Paging_EnablePGBitInCR0();

	asm volatile("add $0xC0000000, %esp");

	Paging_directory[0] = 0;
	Paging_directory[1] = 0;

}
