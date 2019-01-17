#include <x86/kernel/include/gdt.h>
#include <x86/asmc/include/pmode.h>
#include <x86/asmc/include/segments.h>

struct GDTEntry gdt[7];
struct GDTRecord gdtr;

void SetupGDTEntry(struct GDTEntry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {

	entry->limit_low      = (uint16_t)  (limit & 0x0000FFFF); 
	entry->base_low       = (uint16_t)  (base  & 0x0000FFFF);
	entry->base_middle    = (uint8_t ) ((base  & 0x00FF0000) >> 16);
	entry->access         = access;
	entry->limit_flags    = (uint8_t ) ((limit & 0x000F0000) >> 16);
	entry->limit_flags   += (uint8_t ) ((flags & 0x0F)       << 4);
	entry->base_high      = (uint8_t ) ((base  & 0xFF000000) >> 24);

	return;

}

void SetupGDT() {

	SetupGDTEntry(&(gdt[0]), 0x00000000, 0x00000000, 0x00                   , 0x00            );

	SetupGDTEntry(&(gdt[1]), 0x00000000, 0x000FFFFF, KERNEL_CODE_SEG_ACCESS , SEG_GRANULARITY);
	SetupGDTEntry(&(gdt[2]), 0x00000000, 0x000FFFFF, KERNEL_DATA_SEG_ACCESS , SEG_GRANULARITY);
	SetupGDTEntry(&(gdt[3]), 0x00000000, 0x000FFFFF, KERNEL_STACK_SEG_ACCESS, SEG_GRANULARITY);

	SetupGDTEntry(&(gdt[4]), 0x00000000, 0x000FFFFF, USER_CODE_SEG_ACCESS   , SEG_GRANULARITY);
	SetupGDTEntry(&(gdt[5]), 0x00000000, 0x000FFFFF, USER_DATA_SEG_ACCESS   , SEG_GRANULARITY);
	SetupGDTEntry(&(gdt[6]), 0x00000000, 0x000FFFFF, USER_STACK_SEG_ACCESS  , SEG_GRANULARITY);

	gdtr.limit = (sizeof(struct GDTEntry))*7 - 1;
	gdtr.base  = (uintptr_t)gdt;
	
	LoadGDT(&gdtr);

	LoadKernelSegments();

	return;

}
