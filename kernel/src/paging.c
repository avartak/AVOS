#include <kernel/include/paging.h>

void Paging_MapEntry(uint32_t* table, uintptr_t entry_ptr, uint32_t entry_num, uint16_t attr) {
    table[entry_num & 0x03FF] = (entry_ptr & 0xFFFFF000) | (attr & 0x0FFF);
}

uint32_t Paging_GetDirectoryEntry(uintptr_t virtual_address) {
    return (uint32_t)virtual_address >> 22;
}

uint32_t Paging_GetTableEntry(uintptr_t virtual_address) {
    return (uint32_t)virtual_address >> 12 & 0x03FF;
}

bool Paging_TableExists(uintptr_t virtual_address) {
	uint32_t  pdindex = Paging_GetDirectoryEntry(virtual_address);
	uint32_t* pd      = (uint32_t*)0xFFFFF000;

	if (pd[pdindex] == 0) return false;
	else return true;
}

bool Paging_ClearTable(uintptr_t virtual_address) {
	uint32_t  pdindex = Paging_GetDirectoryEntry(virtual_address);
	uint32_t* pd      = (uint32_t*)0xFFFFF000;
	uint32_t* pt      = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);
	
	if (pd[pdindex] == 0) return false;

	for (size_t i = 0; i < 0x400; i++) pt[i] = 0;
    return true;
}

uintptr_t Paging_GetPhysicalAddress(uintptr_t virtual_address) {
	uint32_t  pdindex = Paging_GetDirectoryEntry(virtual_address);
	uint32_t  ptindex = Paging_GetTableEntry(virtual_address);
	uint32_t* pd      =  (uint32_t*)0xFFFFF000;
	uint32_t* pt      = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);

	if (pd[pdindex] == 0 || pt[ptindex] == 0) return (uintptr_t)MEMORY_NULL_PTR;
	return (uintptr_t)(pt[ptindex] & 0xFFFFF000) + (uintptr_t)(virtual_address & 0x00000FFF);
}

bool Paging_MapVirtualPage(uintptr_t virtual_address, uintptr_t phys_address, uint16_t attr) {
    uint32_t  pdindex = Paging_GetDirectoryEntry(virtual_address);
    uint32_t  ptindex = Paging_GetTableEntry(virtual_address);
    uint32_t* pd      = (uint32_t*)0xFFFFF000;
    uint32_t* pt      = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);

    if (pd[pdindex] == 0 || pt[ptindex] != 0) return false;

    pt[ptindex] = phys_address | (attr & 0xFFF) | 0x01;
    return true;
}

bool Paging_UnmapVirtualPage(uintptr_t virtual_address) {
	uint32_t  pdindex = Paging_GetDirectoryEntry(virtual_address);
	uint32_t  ptindex = Paging_GetTableEntry(virtual_address);
	uint32_t* pd      = (uint32_t*)0xFFFFF000;
	uint32_t* pt      = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);
	
	if (pd[pdindex] == 0) return false;

	pt[ptindex] = 0;
	return true;
}

bool Paging_SetupDirectory(struct Process* proc) {
	if (proc == MEMORY_NULL_PTR) return false;

	struct Process_Memory_PagingEntry* pd_map = (proc->memory).paging_directory_map;
	while (pd_map != MEMORY_NULL_PTR) Paging_directory[pd_map->entry] = (uint32_t)(pd_map->address);
	Paging_LoadDirectory((uintptr_t)Paging_directory);
	return true;
}

