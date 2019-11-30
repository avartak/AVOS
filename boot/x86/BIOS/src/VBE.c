#include <boot/x86/BIOS/include/VBE.h>
#include <boot/x86/BIOS/include/bios.h>

struct VBE_Mode_Info VBE_ModeBuffer;

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
	
	return addr + sizeof(struct VBE_Info);
}

bool VBE_GetModeInfo(uint16_t mode, uintptr_t addr) {
	if (mode == 0xFFFF || addr == (uintptr_t)MEMORY_NULL_PTR) return false;

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax   = 0x00004F01;
    BIOS_regs.ecx   = mode;
    BIOS_regs.edi   = (addr & 0xF);
    BIOS_regs.es    = (addr >> 4);

    BIOS_Interrupt(0x10, &BIOS_regs);

	if (BIOS_regs.eax != 0x4F) return false;
	else return true;
}

bool VBE_SetMode(uint16_t mode) {
    if (mode == 0xFFFF) return false;

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax   = 0x00004F02;
    BIOS_regs.ebx   = mode;

    BIOS_Interrupt(0x10, &BIOS_regs);

    if (BIOS_regs.eax != 0x4F) return false;
    else return true;
}

uint16_t VBE_GetCurrentMode() {

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax   = 0x00004F03;
    BIOS_Interrupt(0x10, &BIOS_regs);

	if (BIOS_regs.eax != 0x4F) return 0xFFFF;
	else return (uint16_t)BIOS_regs.ebx;
}

uintptr_t VBE_StorePModeInfo(uintptr_t addr) {

	struct VBE_PMode_Info* pminfo = (struct VBE_PMode_Info*)(addr);

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax   = 0x00004F0A;
    BIOS_regs.ebx   = 0;

    BIOS_Interrupt(0x10, &BIOS_regs);

    if (BIOS_regs.eax != 0x4F || (BIOS_regs.flags & 1) == 1) return addr;
	pminfo->segment = (uint16_t)BIOS_regs.es;
	pminfo->offset  = (uint16_t)BIOS_regs.edi;
	pminfo->length  = (uint16_t)BIOS_regs.ecx;
    return addr + sizeof(struct VBE_Info);

}

uintptr_t VBE_GetModeBuffer() {
	return (uintptr_t)(&VBE_ModeBuffer);
}

uint16_t VBE_GetTextMode(uint16_t* video_modes, uint32_t width, uint32_t height) {
	if (video_modes == MEMORY_NULL_PTR) return 0xFFFF;
	if (width == 0 && height == 0) {
		for (size_t i = 0; video_modes[i] != 0xFFFF; i++) {
			if (video_modes[i] != 3) continue;
			if (!VBE_GetModeInfo(video_modes[i], (uintptr_t)(&VBE_ModeBuffer))) continue;
			if ((VBE_ModeBuffer.attributes & 0x10) == 0 &&  VBE_ModeBuffer.width == 80 && VBE_ModeBuffer.height == 25) return video_modes[i];
		}
		for (size_t i = 0; video_modes[i] != 0xFFFF; i++) {
			if (!VBE_GetModeInfo(video_modes[i], (uintptr_t)(&VBE_ModeBuffer))) continue;
			if ((VBE_ModeBuffer.attributes & 0x10) == 0 && VBE_ModeBuffer.width == 80 && VBE_ModeBuffer.height == 25) return video_modes[i];
		}
		for (size_t i = 0; video_modes[i] != 0xFFFF; i++) {
			if (!VBE_GetModeInfo(video_modes[i], (uintptr_t)(&VBE_ModeBuffer))) continue;
			if ((VBE_ModeBuffer.attributes & 0x10) == 0) return video_modes[i];
		}
	}
	else {
		for (size_t i = 0; video_modes[i] != 0xFFFF; i++) {
			if (!VBE_GetModeInfo(video_modes[i], (uintptr_t)(&VBE_ModeBuffer))) continue;
			if ((VBE_ModeBuffer.attributes & 0x10) == 0 && (width == 0 || VBE_ModeBuffer.width == width) && (height == 0 || VBE_ModeBuffer.height == height)) return video_modes[i];
		}
	}
	return 0xFFFF;
}

uint16_t VBE_GetMode(uint16_t* video_modes, uint32_t width, uint32_t height, uint32_t depth) {
	if (video_modes == MEMORY_NULL_PTR) return 0xFFFF;
	if (width == 0 && height == 0 && depth == 0) {
        for (size_t i = 0; video_modes[i] != 0xFFFF; i++) {
            if (video_modes[i] != 3) continue;
            if (!VBE_GetModeInfo(video_modes[i], (uintptr_t)(&VBE_ModeBuffer))) continue;
            if ((VBE_ModeBuffer.attributes & 0x10) == 0 &&  VBE_ModeBuffer.width == 80 && VBE_ModeBuffer.height == 25) return video_modes[i];
        }
        for (size_t i = 0; video_modes[i] != 0xFFFF; i++) {
            if (!VBE_GetModeInfo(video_modes[i], (uintptr_t)(&VBE_ModeBuffer))) continue;
            if ((VBE_ModeBuffer.attributes & 0x10) == 0 && VBE_ModeBuffer.width == 80 && VBE_ModeBuffer.height == 25) return video_modes[i];
        }
        for (size_t i = 0; video_modes[i] != 0xFFFF; i++) {
            if (!VBE_GetModeInfo(video_modes[i], (uintptr_t)(&VBE_ModeBuffer))) continue;
            return video_modes[i];
        }
	}
	else if ((width != 0 || height != 0) && depth == 0) {
		uint16_t retmode = VBE_GetTextMode(video_modes, width, height);
		if (retmode != 0xFFFF) return retmode;

		for (size_t i = 0; video_modes[i] != 0xFFFF; i++) {
			if (!VBE_GetModeInfo(video_modes[i], (uintptr_t)(&VBE_ModeBuffer))) continue;
			if ((width == 0 || VBE_ModeBuffer.width == width) && (height == 0 || VBE_ModeBuffer.height == height)) return video_modes[i];
		}
	}
	else {
		for (size_t i = 0; video_modes[i] != 0xFFFF; i++) {
			if (!VBE_GetModeInfo(video_modes[i], (uintptr_t)(&VBE_ModeBuffer))) continue;
			if ((width == 0 || VBE_ModeBuffer.width == width) && (height == 0 || VBE_ModeBuffer.height == height) && VBE_ModeBuffer.bpp == depth) return video_modes[i];
		}
	}
	return 0xFFFF;
}

