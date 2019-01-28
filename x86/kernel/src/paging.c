#include <x86/kernel/include/paging.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

uint32_t Paging_kernel_directory[0x400]__attribute__((aligned(0x1000)));
uint32_t Paging_kernel_selftable[0x400]__attribute__((aligned(0x1000)));

uintptr_t Paging_GetPhysicalAddress(uintptr_t virtual_address) {
	uint32_t pdindex = Paging_GetDirectoryEntry(virtual_address);
	uint32_t ptindex = Paging_GetTableEntry(virtual_address);
	
	uint32_t* pd =  (uint32_t*)0xFFFFF000;
	if (pd[pdindex] == 0) return PAGING_NULL_PTR;
	
	uint32_t* pt = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);
	if (pt[ptindex] == 0) return PAGING_NULL_PTR;
	
	return (uintptr_t)(pt[ptindex] & 0xFFFFF000) + (uintptr_t)(virtual_address & 0x00000FFF);
}

extern bool Paging_MapVirtualToPhysicalPage(uintptr_t virtual_address, uintptr_t phys_address, uint16_t attr) {
    uint32_t pdindex = Paging_GetDirectoryEntry(virtual_address);
    uint32_t ptindex = Paging_GetTableEntry(virtual_address);

    uint32_t* pd = (uint32_t*)0xFFFFF000;
    if (pd[pdindex] == 0) return false;

    uint32_t* pt = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);
    if (pt[ptindex] != 0) return false;

    pt[ptindex] = phys_address | (attr & 0xFFF) | 0x01;
    return true;
}

extern bool Paging_UnmapVirtualPage(uintptr_t virtual_address) {
	uint32_t pdindex = Paging_GetDirectoryEntry(virtual_address);
	uint32_t ptindex = Paging_GetTableEntry(virtual_address);
	
	uint32_t* pd = (uint32_t*)0xFFFFF000;
	if (pd[pdindex] == 0) return false;
	
	uint32_t* pt = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);
	
	pt[ptindex] = 0;
	return true;
}

void Paging_Initialize() {
    for (size_t i = 0; i < 1024; i++) Paging_kernel_directory[i] = 0;   
	Paging_MapMemoryBlockInTable(Paging_kernel_selftable, 0, PAGING_KERN_PAGE_FLAGS);

	Paging_MapTableInDirectory(Paging_kernel_directory, Paging_GetPhysicalAddress((uintptr_t)Paging_kernel_selftable), Paging_GetDirectoryEntry(0xC0000000), PAGING_KERN_TABLE_FLAGS);
	Paging_MapTableInDirectory(Paging_kernel_directory, Paging_GetPhysicalAddress((uintptr_t)Paging_kernel_directory), Paging_GetDirectoryEntry(0xFFFFF000), PAGING_KERN_TABLE_FLAGS);

	Paging_LoadDirectory(Paging_GetPhysicalAddress((uintptr_t)Paging_kernel_directory));
	Paging_EnablePGBitInCR0();
}
