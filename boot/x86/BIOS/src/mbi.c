#include <boot/x86/BIOS/include/mbi.h>
#include <boot/x86/BIOS/include/diskio.h>
#include <boot/x86/BIOS/include/RAM.h>
#include <boot/x86/BIOS/include/VBE.h>
#include <boot/x86/BIOS/include/console.h>
#include <boot/x86/BIOS/include/discovery.h>

bool Multiboot_SaveBootLoaderInfo(uintptr_t mbi_addr) {

	struct Multiboot_Info_Name* mbi_name = (struct Multiboot_Info_Name*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME);
	if (mbi_name == MEMORY_NULL_PTR || mbi_name->type != 0) return false;
	
	mbi_name->type = MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME;
	mbi_name->size = 16;
	mbi_name->string[0] = 'A';
	mbi_name->string[1] = 'V';
	mbi_name->string[2] = 'B';
	mbi_name->string[3] = 'L';
	mbi_name->string[4] = '\0';
	
	return Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_name);
}

bool Multiboot_SaveBootCommand(uintptr_t mbi_addr) {

	struct Multiboot_Info_Command* mbi_cmd = (struct Multiboot_Info_Command*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_CMDLINE);
	if (mbi_cmd == MEMORY_NULL_PTR || mbi_cmd->type != 0) return false;
	
	mbi_cmd->type = MULTIBOOT_TAG_TYPE_CMDLINE;
	mbi_cmd->size = 8;
	Console_ReadCommand(mbi_cmd->string);
	size_t i;
	for (i = 0; (mbi_cmd->string)[i] != '\0'; i++) (mbi_cmd->size)++;
	(mbi_cmd->size)++;
	i++;
	for (; (mbi_cmd->size) % 8 != 0; i++) {
		(mbi_cmd->size)++;
		(mbi_cmd->string)[i] = '\0';		
	}
	
	return Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_cmd);
}

bool Multiboot_SaveMemoryMaps(uintptr_t mbi_addr) {

	uintptr_t mem_ram_tag = Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_MMAP);
	if (mem_ram_tag == (uintptr_t)MEMORY_NULL_PTR) return false;
	struct Multiboot_Info_Memory_E820* mbi_mem_ram  = (struct Multiboot_Info_Memory_E820*)mem_ram_tag;
	if (mbi_mem_ram->type != 0) return false;
	
	mbi_mem_ram->size = RAM_StoreE820Info(16 + mem_ram_tag) - mem_ram_tag;
	mbi_mem_ram->type = MULTIBOOT_TAG_TYPE_MMAP;
	mbi_mem_ram->entry_size = 24;
	mbi_mem_ram->entry_version = 0;
	
	Multiboot_TerminateTag(mbi_addr, mem_ram_tag);
	
	struct Multiboot_E820_Entry* mmap = (struct Multiboot_E820_Entry*)(16 + mem_ram_tag);
	size_t mmap_size = (mbi_mem_ram->size - 16)/(mbi_mem_ram->entry_size);
	
	if (mbi_mem_ram->size == 16) return false;
	
	mem_ram_tag = Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_RAM_INFO);
	if (mem_ram_tag == (uintptr_t)MEMORY_NULL_PTR) return false;
	mbi_mem_ram = (struct Multiboot_Info_Memory_E820*)mem_ram_tag;
	if (mbi_mem_ram->type != 0) return false;
	
	mbi_mem_ram->size = RAM_StoreInfo(16 + mem_ram_tag, false, mmap, mmap_size) - mem_ram_tag;
	mbi_mem_ram->type = MULTIBOOT_TAG_TYPE_RAM_INFO;
	mbi_mem_ram->entry_size = sizeof(struct Boot_Block64);
	mbi_mem_ram->entry_version = 0;
	
	Multiboot_TerminateTag(mbi_addr, mem_ram_tag);
	
	mem_ram_tag = Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_RAM_INFO_PAGE_ALIGNED);
	if (mem_ram_tag == (uintptr_t)MEMORY_NULL_PTR) return false;
	mbi_mem_ram = (struct Multiboot_Info_Memory_E820*)mem_ram_tag;
	if (mbi_mem_ram->type != 0) return false;
	
	mbi_mem_ram->size = RAM_StoreInfo(16 + mem_ram_tag, false, mmap, mmap_size) - mem_ram_tag;
	mbi_mem_ram->type = MULTIBOOT_TAG_TYPE_RAM_INFO_PAGE_ALIGNED;
	mbi_mem_ram->entry_size = sizeof(struct Boot_Block64);
	mbi_mem_ram->entry_version = 0;
	
	Multiboot_TerminateTag(mbi_addr, mem_ram_tag);
	
	return true;
}

bool Multiboot_SaveBasicMemoryInfo(uintptr_t mbi_addr) {

	struct Multiboot_Info_Memory_Basic* mbi_mem_basic = (struct Multiboot_Info_Memory_Basic*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);
	if (mbi_mem_basic == MEMORY_NULL_PTR || mbi_mem_basic->type != 0) return false;
	
	mbi_mem_basic->type = MULTIBOOT_TAG_TYPE_BASIC_MEMINFO;
	mbi_mem_basic->size = 16;
	mbi_mem_basic->mem_lower = 0;
	mbi_mem_basic->mem_upper = 0;
	RAM_StoreBasicInfo(8 + (uintptr_t)mbi_mem_basic);
	
	Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_mem_basic);
	
	return true;
}

bool Multiboot_SaveBootDeviceInfo(uintptr_t mbi_addr, struct Boot_Kernel_Info* kernel_info) {

	struct Multiboot_Info_BootDevice* mbi_bootdev = (struct Multiboot_Info_BootDevice*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_BOOTDEV);
	if (mbi_bootdev == MEMORY_NULL_PTR || mbi_bootdev->type != 0) return false;
	
	uint16_t* part = (uint16_t*)(kernel_info->boot_partition);
	for (size_t i = 0; i < 5; i++) {
		if (part[0] == 0xAA55) break;
		part += 8;
	}
	if (part[0] != 0xAA55) return false;
	
	mbi_bootdev->type = MULTIBOOT_TAG_TYPE_BOOTDEV;
	mbi_bootdev->size = 24;
	mbi_bootdev->biosdev = kernel_info->boot_drive_ID;
	mbi_bootdev->partition = 4 - ((uint32_t)part - kernel_info->boot_partition)/16;
	mbi_bootdev->sub_partition = 0xFFFFFFFF;
	
	Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_bootdev);
	
	return true;
}

bool Multiboot_SaveLoadBaseAddress(uintptr_t mbi_addr, struct Boot_Kernel_Info* kernel_info) {

	struct Multiboot_Info_LoadBaseAddress* mbi_mem_base = (struct Multiboot_Info_LoadBaseAddress*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR);
	if (mbi_mem_base == MEMORY_NULL_PTR || mbi_mem_base->type != 0) return false;
	
	mbi_mem_base->type = MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR;
	mbi_mem_base->size = 16;
	mbi_mem_base->load_base_addr = kernel_info->start;
	mbi_mem_base->reserved = 0;
	
	Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_mem_base);
	
	return true;
}


bool Multiboot_SaveGraphicsInfo(uintptr_t mbi_addr, struct Boot_Kernel_Info* kernel_info) {

	uintptr_t multiboot_header_ptr = kernel_info->multiboot_header;
	
	if (multiboot_header_ptr == 0) return false;
	
	bool graphics_mbi_mandatory = false;
	bool EGA_text_supported = false;
	bool vbe_mbi_requested = false;
	bool vbe_mbi_mandatory = false;
	bool framebuffer_mbi_requested = false;
	bool framebuffer_mbi_mandatory = false;
	
	uint32_t width_requested  = 0;
	uint32_t height_requested = 0;
	uint32_t depth_requested  = 0;
	
	struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
	struct Multiboot_Header_Tag* header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);
	
	while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
		bool mandatory = (header_tag->flags & 1) == 0;
		if (header_tag->type == MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST) {
			struct Multiboot_Header_Tag_Information* header_requests = (struct Multiboot_Header_Tag_Information*)header_tag;
			size_t nrequests = (header_requests->size - 8)/sizeof(uint32_t);
			for (size_t i = 0; i < nrequests; i++) {
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_VBE) {
					vbe_mbi_requested = true;
					vbe_mbi_mandatory = mandatory;
				}
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
					framebuffer_mbi_requested = true;
					framebuffer_mbi_mandatory = mandatory;
				}
			}
		}
		else if (header_tag->type == MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS) {
			struct Multiboot_Header_Tag_Console* header_tag_console = (struct Multiboot_Header_Tag_Console*)header_tag;
			if (header_tag_console->console_flags & MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED  ) graphics_mbi_mandatory = true;
			if (header_tag_console->console_flags & MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED) EGA_text_supported = true;
		}
		else if (header_tag->type == MULTIBOOT_HEADER_TAG_FRAMEBUFFER) {
			struct Multiboot_Header_Tag_Framebuffer* header_tag_framebuffer = (struct Multiboot_Header_Tag_Framebuffer*)header_tag;
			width_requested  = header_tag_framebuffer->width;
			height_requested = header_tag_framebuffer->height;
			depth_requested  = header_tag_framebuffer->depth;
		}
		header_tag = (struct Multiboot_Header_Tag*)((uintptr_t)header_tag + header_tag->size);
	}
	
	if (!graphics_mbi_mandatory && !vbe_mbi_requested && !framebuffer_mbi_requested) return true;
	
	struct Multiboot_Info_VBE* mbi_vbe = (struct Multiboot_Info_VBE*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_VBE);
	if (mbi_vbe == MEMORY_NULL_PTR || mbi_vbe->type != 0) return false;
	
	mbi_vbe->type = MULTIBOOT_TAG_TYPE_VBE;
	mbi_vbe->size = 784;
	for (size_t i = 0; i < 0x200; i++) mbi_vbe->vbe_control_info[i] = 0;
	for (size_t i = 0; i < 0x100; i++) mbi_vbe->vbe_mode_info[i] = 0;
	size_t vbe_info_size = VBE_StoreInfo(16 + (uintptr_t)mbi_vbe) - (16 + (uintptr_t)mbi_vbe);
	if (vbe_info_size == 0) {
		mbi_vbe->size = 0;
		Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_vbe);
		if (graphics_mbi_mandatory || vbe_mbi_mandatory || framebuffer_mbi_mandatory) return false;
		return true;
	}
	
	mbi_vbe->vbe_mode = 0xFFFF;	
	mbi_vbe->vbe_interface_seg = 0;	
	mbi_vbe->vbe_interface_off = 0;	
	mbi_vbe->vbe_interface_len = 0;	
	VBE_StorePModeInfo((uintptr_t)(&(mbi_vbe->vbe_interface_seg)));
	
	struct VBE_Info* vbe_info = (struct VBE_Info*)(16 + (uintptr_t)mbi_vbe);
	uintptr_t vmodes = (uintptr_t)(vbe_info->video_modes);
	uint16_t* video_modes = (uint16_t*)(((vmodes >> 16) << 4) + (vmodes & 0xFFFF));
	
	uint16_t preferred_mode;
	if (EGA_text_supported && depth_requested == 0) preferred_mode = VBE_GetTextMode(video_modes, 80, 25);
	if (preferred_mode == 0xFFFF) preferred_mode = VBE_GetMode(video_modes, width_requested, height_requested, depth_requested);
	if (preferred_mode == 0xFFFF) preferred_mode = VBE_GetCurrentMode();
	if (preferred_mode == 0xFFFF || VBE_GetModeInfo(preferred_mode, (uintptr_t)mbi_vbe->vbe_mode_info) == false) {
		mbi_vbe->size = 0;
		Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_vbe);
		if (graphics_mbi_mandatory || vbe_mbi_mandatory || framebuffer_mbi_mandatory) return false;
		return true;
	}
	
	mbi_vbe->vbe_mode = preferred_mode;
	Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_vbe);
	if (!graphics_mbi_mandatory && !framebuffer_mbi_requested) return true;
	
	struct Multiboot_Info_Framebuffer* mbi_fbuf = (struct Multiboot_Info_Framebuffer*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
	if (mbi_fbuf == MEMORY_NULL_PTR || mbi_fbuf->type != 0) return false;
	
	struct VBE_Mode_Info* mode_info = (struct VBE_Mode_Info*)(mbi_vbe->vbe_mode_info);
	mbi_fbuf->type = MULTIBOOT_TAG_TYPE_FRAMEBUFFER;
	mbi_fbuf->framebuffer_addr   = mode_info->framebuffer;
	mbi_fbuf->framebuffer_pitch  = mode_info->pitch;
	mbi_fbuf->framebuffer_width  = mode_info->width;
	mbi_fbuf->framebuffer_height = mode_info->height;
	mbi_fbuf->framebuffer_bpp    = mode_info->bpp;
	mbi_fbuf->framebuffer_type   = ((mode_info->attributes & 0x10) == 0 ? 2 : 1);
	if (mbi_fbuf->framebuffer_type == 2) mbi_fbuf->framebuffer_bpp = 0x10;
	mbi_fbuf->reserved = 0;
	mbi_fbuf->framebuffer_red_field_position   = mode_info->red_position;
	mbi_fbuf->framebuffer_red_mask_size        = mode_info->red_mask;
	mbi_fbuf->framebuffer_green_field_position = mode_info->green_position;
	mbi_fbuf->framebuffer_green_mask_size      = mode_info->green_mask;
	mbi_fbuf->framebuffer_blue_field_position  = mode_info->blue_position;
	mbi_fbuf->framebuffer_blue_mask_size       = mode_info->green_position;
	mbi_fbuf->size = sizeof(*mbi_fbuf);
	
	Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_fbuf);
	
	if (!VBE_SetMode(preferred_mode)) return false;
	return true;
}

bool Multiboot_SaveAPMInfo(uintptr_t mbi_addr) {

	struct Multiboot_Info_APM* mbi_apm = (struct Multiboot_Info_APM*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_APM);
	if (mbi_apm == MEMORY_NULL_PTR || mbi_apm->type != 0) return false;
	
	mbi_apm->type = MULTIBOOT_TAG_TYPE_APM;
	mbi_apm->size = sizeof(struct Multiboot_Info_APM);
	
	struct Multiboot_APM_Interface* iapm = &(mbi_apm->apm_interface);
	if (Discovery_StoreAPMInfo((uintptr_t)iapm) == (uintptr_t)iapm) {
		mbi_apm->size = 0;
		Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_apm);
		return false;
	}
	else {
		mbi_apm->reserved1 = 0;
		mbi_apm->reserved2 = 0;
		Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_apm);
		return true;
	}

}

bool Multiboot_SaveSMBIOSInfo(uintptr_t mbi_addr) {

	struct Multiboot_Info_SMBIOS* mbi_smbios = (struct Multiboot_Info_SMBIOS*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_SMBIOS);
	if (mbi_smbios == MEMORY_NULL_PTR || mbi_smbios->type != 0) return false;
	
	mbi_smbios->type = MULTIBOOT_TAG_TYPE_SMBIOS;
	mbi_smbios->size = Discovery_StoreSMBIOSInfo(8 + (uintptr_t)mbi_smbios) - (uintptr_t)mbi_smbios;
	
	if (mbi_smbios->size <= 8) {
		mbi_smbios->size = 0;
		Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_smbios);
		return false;
	}
	else {
		Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_smbios);
		return true;
	}
}

bool Multiboot_SaveACPIInfo(uintptr_t mbi_addr, bool old) {

	struct Multiboot_Info_ACPIv1* mbi_acpiv1 = (struct Multiboot_Info_ACPIv1*)Multiboot_FindMBITagAddress(mbi_addr, (old ? MULTIBOOT_TAG_TYPE_ACPI_OLD : MULTIBOOT_TAG_TYPE_ACPI_NEW));
	if (mbi_acpiv1 == MEMORY_NULL_PTR || mbi_acpiv1->type != 0) return false;
	
	mbi_acpiv1->type = (old ? MULTIBOOT_TAG_TYPE_ACPI_OLD : MULTIBOOT_TAG_TYPE_ACPI_NEW);
	mbi_acpiv1->size = (old ? sizeof(struct Multiboot_Info_ACPIv1) : sizeof(struct Multiboot_Info_ACPIv2));
	
	if (Discovery_StoreACPIInfo(8 + (uintptr_t)mbi_acpiv1, old) == 8 + (uintptr_t)mbi_acpiv1) {
		mbi_acpiv1->size = 0;
		Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_acpiv1);
		return false;
	}
	else {
		Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_acpiv1);
		return true;
	}
}

bool Multiboot_SaveInfo(uintptr_t mbi_addr, struct Boot_Kernel_Info* kernel_info) {

	uintptr_t multiboot_header_ptr = kernel_info->multiboot_header;
	
	struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
	struct Multiboot_Header_Tag* header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);
	
	while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
		bool quit = (header_tag->flags & 1) == 0;
		if (header_tag->type == MULTIBOOT_HEADER_TAG_EFI_BS             ) {if (quit) return false;}
		if (header_tag->type == MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32) {if (quit) return false;}
		if (header_tag->type == MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64) {if (quit) return false;}
		if (header_tag->type == MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST) {
			struct Multiboot_Header_Tag_Information* header_requests = (struct Multiboot_Header_Tag_Information*)header_tag;
			size_t nrequests = (header_requests->size - 8)/sizeof(uint32_t);
			for (size_t i = 0; i < nrequests; i++) {
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_CMDLINE         ) Multiboot_SaveBootCommand(mbi_addr);
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME) Multiboot_SaveBootLoaderInfo(mbi_addr);
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_BASIC_MEMINFO   ) Multiboot_SaveBasicMemoryInfo(mbi_addr);
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_BOOTDEV         ) Multiboot_SaveBootDeviceInfo(mbi_addr, kernel_info);
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_APM             ) Multiboot_SaveAPMInfo(mbi_addr);
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_SMBIOS          ) Multiboot_SaveSMBIOSInfo(mbi_addr);
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_ACPI_OLD        ) Multiboot_SaveACPIInfo(mbi_addr, true);
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_ACPI_NEW        ) Multiboot_SaveACPIInfo(mbi_addr, false);
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR  ) Multiboot_SaveLoadBaseAddress(mbi_addr, kernel_info);
				
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_EFI32           ) {if (quit) return false;}
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_EFI64           ) {if (quit) return false;}
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_EFI_MMAP        ) {if (quit) return false;}
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_EFI_BS          ) {if (quit) return false;}
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_EFI32_IH        ) {if (quit) return false;}
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_EFI64_IH        ) {if (quit) return false;}
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_NETWORK         ) {if (quit) return false;}
			}
		}
		header_tag = (struct Multiboot_Header_Tag*)((uintptr_t)header_tag + header_tag->size);
	}
	
	if (!Multiboot_SaveGraphicsInfo(mbi_addr, kernel_info)) return false;
	
	return true;
}

