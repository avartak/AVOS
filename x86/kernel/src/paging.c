#include <x86/kernel/include/paging.h>

#include <stdint.h>
#include <stddef.h>

uint32_t page_directory[0x400]__attribute__((aligned(0x1000)));

void AddPageTableToDirectory(uintptr_t pt, uint32_t entry, uint16_t attr) {

	entry &= 0x03FF; 
	attr  &= 0x0FFF; 
	pt    &= 0xFFFFF000;
	pt    |= attr;

	page_directory[entry] = pt;

}

void MapPageTableTo4MBMemoryChunk(uintptr_t pt, uintptr_t addr, uint16_t attr) {

	uint32_t* table = (uint32_t*)pt;

	attr &= 0x0FFF; 
	addr &= 0xFFFFF000;
	addr |= attr;

    for(size_t i = 0; i < 1024; i++) {
        table[i] = i * 0x1000 + addr;
    }
	

}

void InitPaging() {

	for (size_t i = 0; i < 1024; i++) page_directory[i] = 0;	

	uintptr_t ktable = LOC_PAGEMAP_PM;
	uintptr_t ptable = LOC_PAGEMAP_PM + 0x1000;

	MapPageTableTo4MBMemoryChunk(ktable,              0, 3);
	MapPageTableTo4MBMemoryChunk(ptable, LOC_PAGEMAP_PM, 3);

	AddPageTableToDirectory(ktable,   0, 3);
	AddPageTableToDirectory(ptable,   1, 3);
	AddPageTableToDirectory(ktable, 768, 3);
	AddPageTableToDirectory(ptable, 769, 3);

	LoadPageDirectory((uint32_t)page_directory - LOC_KERNEL_HH_OFFSET);
	EnablePGBitInCR0();

	asm volatile("add $0xC0000000, %esp");

	page_directory[0] = 0;
	page_directory[1] = 0;
}
