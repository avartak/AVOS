#include <x86/kernel/include/paging.h>

uint32_t Paging_directory[0x400]__attribute__((aligned(0x1000)));
uint32_t Paging_kerntable[0x400]__attribute__((aligned(0x1000)));

uintptr_t Paging_GetPhysicalAddress(uintptr_t virtual_address) {
	uint32_t pdindex = Paging_GetDirectoryEntry(virtual_address);
	uint32_t ptindex = Paging_GetTableEntry(virtual_address);
	
	uint32_t* pd =  (uint32_t*)0xFFFFF000;
	if (pd[pdindex] == 0) return PAGING_NULL_PTR;
	
	uint32_t* pt = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);
	if (pt[ptindex] == 0) return PAGING_NULL_PTR;
	
	return (uintptr_t)(pt[ptindex] & 0xFFFFF000) + (uintptr_t)(virtual_address & 0x00000FFF);
}

bool Paging_TableExists(uintptr_t virtual_address) {
	uint32_t pdindex = Paging_GetDirectoryEntry(virtual_address);
	uint32_t* pd = (uint32_t*)0xFFFFF000;
	if (pd[pdindex] == 0) return false;
	else return true;
}

bool Paging_ClearTable(uintptr_t virtual_address) {
	if (!Paging_TableExists(virtual_address)) return false;

    uint32_t  pdindex = Paging_GetDirectoryEntry(virtual_address);
    uint32_t* pt = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);
	for (size_t i = 0; i < 0x400; i++) pt[i] = 0;
    return true;

}

bool Paging_MapVirtualToPhysicalPage(uintptr_t virtual_address, uintptr_t phys_address, uint16_t attr) {
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
	uintptr_t ipd = (uintptr_t)Paging_directory;
	uintptr_t ipt = (uintptr_t)Paging_kerntable;

	if (ipd > 0xC0000000) ipd -= 0xC0000000;
	if (ipt > 0xC0000000) ipt -= 0xC0000000;

	uint32_t* pd = (uint32_t*)ipd;
	uint32_t* pt = (uint32_t*)ipt;

	for (size_t i = 0; i < 0x400; i++) pd[i] = 0;	
	for (size_t i = 0; i < 0x400; i++) pt[i] = 0;	

	Paging_MapMemoryBlockInTable(pt, 0, PAGING_KERN_PAGE_FLAGS);

	Paging_MapTableInDirectory(pd, ipt,    0, PAGING_KERN_PAGE_FLAGS);
	Paging_MapTableInDirectory(pd, ipt,  768, PAGING_KERN_PAGE_FLAGS);
	Paging_MapTableInDirectory(pd, ipd, 1023, PAGING_KERN_PAGE_FLAGS);

	Paging_LoadDirectory(ipd);
	Paging_EnablePGBitInCR0();
}
