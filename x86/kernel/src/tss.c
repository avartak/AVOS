#include <x86/kernel/include/tss.h>

#include <stddef.h>

struct TSS_Segment TSS_seg;

void TSS_Initialize() {

	TSS_seg.ss0 = TSS_KERN_DATA_SEG;
	TSS_seg.esp = 0xC0400000;

	for (size_t i = 0; i < 0x2000; i++) TSS_seg.ioport_map[i] = 0xFF;

	uint16_t tss_seg_desc = TSS_KERN_TSS_SEG;
	asm volatile("ltr %%ax" : : "a"(tss_seg_desc) : );

    return;

}

