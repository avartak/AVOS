#include <x86/kernel/include/gdt.h>
#include <x86/kernel/include/tss.h>

struct GDT_Entry      GDT_entries[6];
struct GDT_Descriptor GDT_desc;

void GDT_SetupEntry(struct GDT_Entry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {

	entry->limit_low      = (uint16_t)  (limit & 0x0000FFFF); 
	entry->base_low       = (uint16_t)  (base  & 0x0000FFFF);
	entry->base_middle    = (uint8_t ) ((base  & 0x00FF0000) >> 16);
	entry->access         = access;
	entry->limit_flags    = (uint8_t ) ((limit & 0x000F0000) >> 16);
	entry->limit_flags   += (uint8_t ) ((flags & 0x0F)       << 4);
	entry->base_high      = (uint8_t ) ((base  & 0xFF000000) >> 24);

	return;

}

void GDT_Initialize() {

	uint32_t tss_seg_base  = (uint32_t) &TSS_seg;
	uint32_t tss_seg_limit = sizeof(TSS_seg) - 1;
	tss_seg_base -= 0xC0000000;

	GDT_SetupEntry(&(GDT_entries[0]), 0x00000000  , 0x00000000   , 0x00                    , 0x00               );
	GDT_SetupEntry(&(GDT_entries[1]), 0x00000000  , 0x000FFFFF   , GDT_KERN_CODE_SEG_ACCESS, GDT_SEG_GRANULARITY);
	GDT_SetupEntry(&(GDT_entries[2]), 0x00000000  , 0x000FFFFF   , GDT_KERN_DATA_SEG_ACCESS, GDT_SEG_GRANULARITY);
	GDT_SetupEntry(&(GDT_entries[3]), 0x00000000  , 0x000FFFFF   , GDT_USER_CODE_SEG_ACCESS, GDT_SEG_GRANULARITY);
	GDT_SetupEntry(&(GDT_entries[4]), 0x00000000  , 0x000FFFFF   , GDT_USER_DATA_SEG_ACCESS, GDT_SEG_GRANULARITY);
	GDT_SetupEntry(&(GDT_entries[5]), tss_seg_base, tss_seg_limit, GDT_TSS_SEG_TYPE_ACCESS , GDT_SEG_GRANULARITY);

	GDT_desc.limit = (sizeof(struct GDT_Entry))*6 - 1;
	GDT_desc.base  = (uintptr_t)GDT_entries;
	
	GDT_Load(&GDT_desc);

	GDT_LoadKernelSegments();

	return;

}

