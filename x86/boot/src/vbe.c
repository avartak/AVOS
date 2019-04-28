#include <x86/boot/include/vbe.h>
#include <x86/boot/include/bios.h>

uintptr_t VBE_StoreInfo(uintptr_t addr) {

	struct VBE_Info* vinfo = (struct VBE_Info*)(addr);
	(vinfo->signature)[0] = 'V';
	(vinfo->signature)[1] = 'B';
	(vinfo->signature)[2] = 'E';
	(vinfo->signature)[3] = '2';

    struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax   = 0x00004F00;
    BIOS_regs.edi   = (addr & 0xF);
    BIOS_regs.es    = (addr >> 4);

	BIOS_Interrupt(0x10, &BIOS_regs);

	if (BIOS_regs.eax != 0x4F || (BIOS_regs.flags & 1) == 1) return addr;
	if ((vinfo->signature)[0] != 'V' || (vinfo->signature)[1] != 'E' || (vinfo->signature)[2] != 'S' || (vinfo->signature)[3] != 'A') return addr;

	uintptr_t vmodes = (uintptr_t)(vinfo->video_modes);
	uint16_t* video_modes = (uint16_t*)(((vmodes >> 16) << 4) + (vmodes & 0xFFFF));

	struct VBE_Mode_Info* vmodeinfo = (struct VBE_Mode_Info*)(addr+0x100);
	for (size_t i = 0; video_modes[i] != 0xFFFF; i++) {
		BIOS_regs.eax = 0x00004F01;
    	BIOS_regs.ecx = video_modes[i];
    	BIOS_regs.edi = ((addr+0x200) & 0xF);
    	BIOS_regs.es  = ((addr+0x200) >> 4);

		BIOS_Interrupt(0x10, &BIOS_regs);
		if (BIOS_regs.eax != 0x4F || (BIOS_regs.flags & 1) == 1) return addr+0x100;
		vmodeinfo++;
	}
	vmodeinfo++;

	return (uintptr_t)vmodeinfo;

}

