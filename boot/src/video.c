#include <boot/include/video.h>
#include <boot/include/bios.h>

// Get the VBE information structure
// This contains a pointer to the list of available video modes
uint32_t VBE_StoreInfo(uint32_t addr) {

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
	
	if (BIOS_regs.eax != 0x4F) return addr;
	if ((vinfo->signature)[0] != 'V' || (vinfo->signature)[1] != 'E' || (vinfo->signature)[2] != 'S' || (vinfo->signature)[3] != 'A') return addr;
	
	return addr + sizeof(struct VBE_Info);
}

// Get information about a certain video mode
bool VBE_GetModeInfo(uint16_t mode, uint32_t addr) {

	if (mode == 0xFFFF || addr == (uint32_t)MEMORY_NULL_PTR) return false;
	
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

// Set the video mode to a specific choice
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

// Get the current video mode
uint16_t VBE_GetCurrentMode() {

	struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);
	
	BIOS_regs.eax   = 0x00004F03;
	BIOS_Interrupt(0x10, &BIOS_regs);
	
	if (BIOS_regs.eax != 0x4F) return 0xFFFF;
	else return (uint16_t)BIOS_regs.ebx;
}

// Pointers to certain code/functions that can be run from protected mode
uint32_t VBE_StorePModeInfo(uint32_t addr) {

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

// Get the text mode with the desired attributes
// * width  : Number of characters in a line
// * height : Number of lines
// If width and height are set to 0
// * We first try to find the 'standard' 80x25 text mode 3  
// * If not found, we try to find any available 80x25 text mode
// * If not found, we try to find any available text mode
// If either width or height are non-zero then we find the first available text mode with matching attributes (0 means any)
// Note : A video mode is a text mode if the 5th least significant bit of the attributes word is clear (graphics mode if it is set) 
uint16_t VBE_GetTextMode(uint16_t* video_modes, uint32_t width, uint32_t height) {

	if (video_modes == MEMORY_NULL_PTR) return 0xFFFF;
	
	struct VBE_Mode_Info VBE_ModeBuffer;
	if (width == 0 && height == 0) {
		for (uint32_t i = 0; video_modes[i] != 0xFFFF; i++) {
			if (video_modes[i] != 3) continue;
			if (!VBE_GetModeInfo(video_modes[i], (uint32_t)(&VBE_ModeBuffer))) continue;
			if ((VBE_ModeBuffer.attributes & 0x10) == 0 &&  VBE_ModeBuffer.width == 80 && VBE_ModeBuffer.height == 25) return video_modes[i];
		}
		for (uint32_t i = 0; video_modes[i] != 0xFFFF; i++) {
			if (!VBE_GetModeInfo(video_modes[i], (uint32_t)(&VBE_ModeBuffer))) continue;
			if ((VBE_ModeBuffer.attributes & 0x10) == 0 && VBE_ModeBuffer.width == 80 && VBE_ModeBuffer.height == 25) return video_modes[i];
		}
		for (uint32_t i = 0; video_modes[i] != 0xFFFF; i++) {
			if (!VBE_GetModeInfo(video_modes[i], (uint32_t)(&VBE_ModeBuffer))) continue;
			if ((VBE_ModeBuffer.attributes & 0x10) == 0) return video_modes[i];
		}
	}
	else {
		for (uint32_t i = 0; video_modes[i] != 0xFFFF; i++) {
			if (!VBE_GetModeInfo(video_modes[i], (uint32_t)(&VBE_ModeBuffer))) continue;
			if ((VBE_ModeBuffer.attributes & 0x10) == 0 && (width == 0 || VBE_ModeBuffer.width == width) && (height == 0 || VBE_ModeBuffer.height == height)) return video_modes[i];
		}
	}
	return 0xFFFF;
}

// Get the video mode with the desired attributes
// * width  : width in pixels
// * height : height in pixels
// * depth  : color bits per pixel
// If width, height and depth are set to 0
// * We first try to find the 'standard' 80x25 text mode 3  
// * If not found, we try to find any available 80x25 text mode
// * If not found, we try to find any available text mode
// * If not found, we try to find any available mode
// If width or height are nonzero but depth is 0
// * We first try to find the text mode consistent with the width and height attributes (0 means any)
// * If not found, we try to find any mode consistent with the width and height attributes (0 means any)
// If depth is nonzero
// * We try to find any mode consistent with the width, height and depth attributes (0 means any)
uint16_t VBE_GetMode(uint16_t* video_modes, uint32_t width, uint32_t height, uint32_t depth) {

	if (video_modes == MEMORY_NULL_PTR) return 0xFFFF;
	
	struct VBE_Mode_Info VBE_ModeBuffer;
	if (width == 0 && height == 0 && depth == 0) {
		for (uint32_t i = 0; video_modes[i] != 0xFFFF; i++) {
		    if (video_modes[i] != 3) continue;
		    if (!VBE_GetModeInfo(video_modes[i], (uint32_t)(&VBE_ModeBuffer))) continue;
		    if ((VBE_ModeBuffer.attributes & 0x10) == 0 &&  VBE_ModeBuffer.width == 80 && VBE_ModeBuffer.height == 25) return video_modes[i];
		}
		for (uint32_t i = 0; video_modes[i] != 0xFFFF; i++) {
		    if (!VBE_GetModeInfo(video_modes[i], (uint32_t)(&VBE_ModeBuffer))) continue;
		    if ((VBE_ModeBuffer.attributes & 0x10) == 0 && VBE_ModeBuffer.width == 80 && VBE_ModeBuffer.height == 25) return video_modes[i];
		}
		for (uint32_t i = 0; video_modes[i] != 0xFFFF; i++) {
		    if (!VBE_GetModeInfo(video_modes[i], (uint32_t)(&VBE_ModeBuffer))) continue;
		    if ((VBE_ModeBuffer.attributes & 0x10) == 0) return video_modes[i];
		}
		for (uint32_t i = 0; video_modes[i] != 0xFFFF; i++) {
		    if (!VBE_GetModeInfo(video_modes[i], (uint32_t)(&VBE_ModeBuffer))) continue;
		    return video_modes[i] | 0x4000;
		}
	}
	else if ((width != 0 || height != 0) && depth == 0) {
		uint16_t retmode = VBE_GetTextMode(video_modes, width, height);
		if (retmode != 0xFFFF) return retmode;
		
		for (uint32_t i = 0; video_modes[i] != 0xFFFF; i++) {
			if (!VBE_GetModeInfo(video_modes[i], (uint32_t)(&VBE_ModeBuffer))) continue;
			if ((width == 0 || VBE_ModeBuffer.width == width) && (height == 0 || VBE_ModeBuffer.height == height)) return video_modes[i] | 0x4000;
		}
	}
	else {
		for (uint32_t i = 0; video_modes[i] != 0xFFFF; i++) {
			if (!VBE_GetModeInfo(video_modes[i], (uint32_t)(&VBE_ModeBuffer))) continue;
			if ((width == 0 || VBE_ModeBuffer.width == width) && (height == 0 || VBE_ModeBuffer.height == height) && VBE_ModeBuffer.bpp == depth) return video_modes[i] | 0x4000;
		}
	}
	return 0xFFFF;
}

