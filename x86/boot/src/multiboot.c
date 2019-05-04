#include <x86/boot/include/multiboot.h>
#include <x86/boot/include/elf.h>
#include <x86/boot/include/e820.h>
#include <x86/boot/include/ram.h>
#include <x86/boot/include/vbe.h>

extern void PrintNum(uint32_t num, uint8_t line, uint8_t column);

uintptr_t Multiboot_GetHeader(uintptr_t start_addr, size_t size) {

    uint32_t* start_ptr_u32 = (uint32_t*)start_addr;

    uintptr_t multiboot_header_ptr = start_addr;
    bool found_header_magic = false;
    for (size_t i = 0; i < size/4 && i < 0x2000; i++) {
        if (i % 2 != 0) continue;
        if (start_ptr_u32[i] == 0xE85250D6) {
            multiboot_header_ptr = (uintptr_t)(start_ptr_u32 + i);
            found_header_magic = true;
        }
    }

	if (!found_header_magic) return 0;

	struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
	if (header_magic->architecture != MULTIBOOT_ARCHITECTURE_I386) return 0;
	if (header_magic->header_length < sizeof(struct Multiboot_Header_Magic_Fields)) return 0;
	if ((uint32_t)(header_magic->checksum + header_magic->magic + header_magic->architecture + header_magic->header_length) != 0) return 0;

	return multiboot_header_ptr;

}

size_t Multiboot_LoadKernel(uintptr_t image, size_t file_size, uintptr_t start_addr, uintptr_t mbi_addr) {

	uintptr_t multiboot_header_ptr = Multiboot_GetHeader(image, file_size);
	if (multiboot_header_ptr == 0) return 0;

	struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
	struct Multiboot_Header_Tag* header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);

    while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
        if (header_tag->type == MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST) {
            struct Multiboot_Header_Tag_Information* header_requests = (struct Multiboot_Header_Tag_Information*)header_tag;
            size_t nrequests = (header_requests->size - 8)/sizeof(uint32_t);
            for (size_t i = 0; i < nrequests; i++) {
                if (header_requests->requests[i] == 9) {
					if (!Multiboot_SaveELFSectionHeaders(mbi_addr, image)) return 0;
				}
            }
        }
        header_tag = (struct Multiboot_Header_Tag*)((uintptr_t)header_tag + header_tag->size);
    }


	bool      kernel_loaded = false;
	uintptr_t load_start = image;
	size_t    load_size = file_size;
	size_t    bss_size = 0;

	header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);
	while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
		if (header_tag->type == MULTIBOOT_HEADER_TAG_ADDRESS) {
			struct Multiboot_Header_Tag_Address* header_tag_address = (struct Multiboot_Header_Tag_Address*)header_tag;
			uintptr_t load_header_addr = header_tag_address->header_addr;

			if (header_tag_address->load_addr != 0 && header_tag_address->load_addr != start_addr) return 0;

			load_start = (header_tag_address->load_addr     == (uint32_t)(-1) ? image     : multiboot_header_ptr - (header_tag_address->header_addr - header_tag_address->load_addr));
			load_size  = (header_tag_address->load_end_addr ==             0  ? file_size : header_tag_address->load_end_addr - header_tag_address->load_addr);
			bss_size   = (header_tag_address->bss_end_addr  ==             0  ? 0         : header_tag_address->bss_end_addr - header_tag_address->load_end_addr);

			memmove((uint8_t*)start_addr, (uint8_t*)load_start, load_size);
			memset((uint8_t*)(start_addr + load_size), 0, bss_size);
			
			kernel_loaded = true;
			header_tag = (struct Multiboot_Header_Tag*)((uintptr_t)header_tag + load_header_addr - multiboot_header_ptr);
			break;
		}
		header_tag = (struct Multiboot_Header_Tag*)((uintptr_t)header_tag + header_tag->size);
	}
	if (!kernel_loaded) {
		if (Elf32_IsValidiStaticExecutable(image)) {
			load_size = Elf32_LoadStaticExecutable(image, start_addr);
			if (load_size > 0) kernel_loaded = true;
		}
		else {
			memmove((uint8_t*)start_addr, (uint8_t*)load_start, load_size);
			kernel_loaded = true;
		}
	}

	if (kernel_loaded) {
		uintptr_t end_addr = start_addr + load_size + bss_size;
		uintptr_t image_cleanup_start = (end_addr > image ? end_addr : image);
		if (image_cleanup_start < image + file_size) memset((uint8_t*)image_cleanup_start, 0, image + file_size - image_cleanup_start);

    	return load_size + bss_size;
	}
	else return 0;
	
}

uintptr_t Multiboot_GetKernelEntry(uintptr_t start_addr, uintptr_t multiboot_header_ptr) {
	
	struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
	struct Multiboot_Header_Tag* header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);
	
	uintptr_t kernel_entry = start_addr;	

	while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
		if (header_tag->type == MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS) {
			struct Multiboot_Header_Tag_Entry_Address* header_tag_entry = (struct Multiboot_Header_Tag_Entry_Address*)header_tag;
			kernel_entry = header_tag_entry->entry_addr;
		}
		header_tag = (struct Multiboot_Header_Tag*)((uintptr_t)header_tag + header_tag->size);
	}
	
	return kernel_entry;
}

bool Multiboot_CheckForValidMBI(uintptr_t mbi_addr) {
    struct Multiboot_Info_Start* mbi_start = (struct Multiboot_Info_Start*)mbi_addr;

    struct Multiboot_Info_Tag* mbi_term_tag = (struct Multiboot_Info_Tag*)(mbi_start->total_size + (uintptr_t)mbi_addr - 8);
    if ((mbi_addr % 8) == 0 && mbi_start->reserved == 0 && mbi_start->total_size >= 16 && mbi_term_tag->type == 0 && mbi_term_tag->size == 8) return true;
    else return false;
}

bool Multiboot_CreateMBI(uintptr_t mbi_addr) {

	if (Multiboot_CheckForValidMBI(mbi_addr)) return true;
	if (mbi_addr % 8 != 0) return false; 

    struct Multiboot_Info_Start* mbi_start = (struct Multiboot_Info_Start*)mbi_addr;
	mbi_start->total_size = 16;
	mbi_start->reserved   = 0;

    struct Multiboot_Info_Tag* mbi_term_tag = (struct Multiboot_Info_Tag*)(mbi_start->total_size + (uintptr_t)mbi_addr - 8);
	mbi_term_tag->type = 0;
	mbi_term_tag->size = 8;

	return true;
}

uintptr_t Multiboot_FindMBITagAddress(uintptr_t mbi_addr, uint32_t tag_type) {

	if (!Multiboot_CheckForValidMBI(mbi_addr)) return (uintptr_t)MEMORY_NULL_PTR;

    struct Multiboot_Info_Start* mbi_start = (struct Multiboot_Info_Start*)mbi_addr;
	struct Multiboot_Info_Tag*   mbi_tag   = (struct Multiboot_Info_Tag*)(mbi_addr + 8);
	struct Multiboot_Info_Tag*   ret_tag   = (struct Multiboot_Info_Tag*)(mbi_addr + 8);

	while (mbi_tag->type != 0 && (uintptr_t)mbi_tag < mbi_addr + mbi_start->total_size) {
		if (mbi_tag->type == tag_type) {
			ret_tag = mbi_tag;
			break;
		}
		mbi_tag = (struct Multiboot_Info_Tag*)(mbi_tag->size + (uintptr_t)mbi_tag);
	}

	if (ret_tag->type == tag_type) return (uintptr_t)ret_tag;
	ret_tag = (struct Multiboot_Info_Tag*)(mbi_start->total_size + (uintptr_t)mbi_addr - 8);
	if (ret_tag->type != 0 || ret_tag->size != 8) return (uintptr_t)MEMORY_NULL_PTR;
	else return (uintptr_t)ret_tag;
}

bool Multiboot_SaveBootLoaderInfo(uintptr_t mbi_addr) {
	if (!Multiboot_CheckForValidMBI(mbi_addr)) return	false;
    struct Multiboot_Info_Start* mbi_start = (struct Multiboot_Info_Start*)mbi_addr;

	uintptr_t name_tag = Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME);
	if (name_tag == (uintptr_t)MEMORY_NULL_PTR) return false;
	struct Multiboot_Info_Name* mbi_name = (struct Multiboot_Info_Name*)name_tag;
	if (mbi_name->type != 0) return false;

    mbi_name->type = MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME;
    mbi_name->size = 16;
    mbi_name->string[0] = 'A';
    mbi_name->string[1] = 'V';
    mbi_name->string[2] = 'B';
    mbi_name->string[3] = 'L';
    mbi_name->string[4] = '\0';

	struct Multiboot_Info_Tag* mbi_term = (struct Multiboot_Info_Tag*)(mbi_name->size + (uintptr_t)mbi_name);
	mbi_term->type = 0;
	mbi_term->size = 8;

	mbi_start->total_size += mbi_name->size;	
	return true;
}

bool Multiboot_SaveMemoryInfo(uintptr_t mbi_addr) {

    if (!Multiboot_CheckForValidMBI(mbi_addr)) return false;
    struct Multiboot_Info_Start* mbi_start = (struct Multiboot_Info_Start*)mbi_addr;

    uintptr_t mem_basic_tag = Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);
    if (mem_basic_tag == (uintptr_t)MEMORY_NULL_PTR) return false;
	struct Multiboot_Info_Memory_Basic* mbi_mem_basic = (struct Multiboot_Info_Memory_Basic*)mem_basic_tag;
    if (mbi_mem_basic->type != 0) return false;

    mbi_mem_basic->type = MULTIBOOT_TAG_TYPE_BASIC_MEMINFO;
    mbi_mem_basic->size = 16;
    mbi_mem_basic->mem_lower = 0;
    mbi_mem_basic->mem_upper = 0;

	struct Multiboot_Info_Tag* mbi_term = (struct Multiboot_Info_Tag*)(mbi_mem_basic->size + (uintptr_t)mbi_mem_basic);
	mbi_term->type = 0;
	mbi_term->size = 8;

	mbi_start->total_size += mbi_mem_basic->size;	

	uintptr_t mem_e820_tag = Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_MMAP);
	if (mem_e820_tag == (uintptr_t)MEMORY_NULL_PTR) return false;
	struct Multiboot_Info_Memory_E820* mbi_mem_e820 = (struct Multiboot_Info_Memory_E820*)mem_e820_tag;
    if (mbi_mem_e820->type != 0) return false;

	mbi_mem_e820->size = E820_StoreInfo(16 + (uintptr_t)mbi_mem_e820) - (uintptr_t)mbi_mem_e820;
	mbi_mem_e820->type = MULTIBOOT_TAG_TYPE_MMAP;
	mbi_mem_e820->entry_size = 24;
	mbi_mem_e820->entry_version = 0;

	mbi_term = (struct Multiboot_Info_Tag*)(mbi_mem_e820->size + (uintptr_t)mbi_mem_e820);
	mbi_term->type = 0;
	mbi_term->size = 8;

	mbi_start->total_size += mbi_mem_e820->size;	

	uintptr_t mem_ram_tag = Multiboot_FindMBITagAddress(mbi_addr, 0x80);
	if (mem_ram_tag == (uintptr_t)MEMORY_NULL_PTR) return false;
	struct Multiboot_Info_Memory_E820* mbi_mem_ram = (struct Multiboot_Info_Memory_E820*)mem_ram_tag;
    if (mbi_mem_ram->type != 0) return false;

	mbi_mem_ram->size = RAM_StoreInfo(16 + (uintptr_t)mbi_mem_ram) - (uintptr_t)mbi_mem_ram;
	mbi_mem_ram->type = 0x80;
	mbi_mem_ram->entry_size = sizeof(struct Info_Entry);
	mbi_mem_ram->entry_version = 0;

	mbi_term = (struct Multiboot_Info_Tag*)(mbi_mem_ram->size + (uintptr_t)mbi_mem_ram);
	mbi_term->type = 0;
	mbi_term->size = 8;

	mbi_start->total_size += mbi_mem_ram->size;	

	struct Info_Entry* ram_table_ptr = (struct Info_Entry*)(16 + (uintptr_t)mbi_mem_ram);
	size_t ram_table_size = (mbi_mem_ram->size - 16)/sizeof(struct Info_Entry);
	for (size_t i = 0; i < ram_table_size; i++) {
		if (ram_table_ptr[i].address <  0x00100000 && ram_table_ptr[i].address + ram_table_ptr[i].size <  0x00100000) mbi_mem_basic->mem_lower += ram_table_ptr[i].size;
		if (ram_table_ptr[i].address <  0x00100000 && ram_table_ptr[i].address + ram_table_ptr[i].size >= 0x00100000) mbi_mem_basic->mem_lower += 0x00100000 - ram_table_ptr[i].address;
		if (ram_table_ptr[i].address >= 0x00100000 && ram_table_ptr[i].address + ram_table_ptr[i].size <= 0xFFFFFFFF) mbi_mem_basic->mem_upper += ram_table_ptr[i].size;
		if (ram_table_ptr[i].address >= 0x00100000 && ram_table_ptr[i].address + ram_table_ptr[i].size >  0xFFFFFFFF) mbi_mem_basic->mem_upper += 0xFFFFFFFF - ram_table_ptr[i].address + 1;
	}

	return true;
}

bool Multiboot_SaveGraphicsInfo(uintptr_t mbi_addr, uintptr_t multiboot_header_ptr) {

	if (mbi_addr == 0 || multiboot_header_ptr == 0) return false;

    if (!Multiboot_CheckForValidMBI(mbi_addr)) return false;
    struct Multiboot_Info_Start* mbi_start = (struct Multiboot_Info_Start*)mbi_addr;

	bool EGA_text_supported = false;
	bool graphics_mbi_mandatory = false;

	uint32_t width_requested  = 0;
	uint32_t height_requested = 0;
	uint32_t depth_requested  = 0;

    struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
    struct Multiboot_Header_Tag* header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);

    while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
        if (header_tag->type == MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS) {
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

    uintptr_t mem_vbe_tag = Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_VBE);
    if (mem_vbe_tag == (uintptr_t)MEMORY_NULL_PTR) return false;
	struct Multiboot_Info_VBE* mbi_vbe = (struct Multiboot_Info_VBE*)mem_vbe_tag;
    if (mbi_vbe->type != 0) return false;

	mbi_vbe->type = MULTIBOOT_TAG_TYPE_VBE;
	mbi_vbe->size = 784;
	size_t vbe_info_size = VBE_StoreInfo(16 + (uintptr_t)mbi_vbe) - (16 + (uintptr_t)mbi_vbe);
	struct Multiboot_Info_Tag* mbi_term = (struct Multiboot_Info_Tag*)(mbi_vbe->size + (uintptr_t)mbi_vbe);
	if (vbe_info_size == 0) {
		mbi_term = (struct Multiboot_Info_Tag*)mbi_vbe;
		mbi_term->type = 0;
		mbi_term->size = 8;
		if (graphics_mbi_mandatory) return false;
		else return true;
	}
	for (size_t i = 0; i < 0x100; i++) mbi_vbe->vbe_mode_info[i] = 0;

	mbi_term->type = 0;
	mbi_term->size = 8;
	mbi_start->total_size += mbi_vbe->size;

	mbi_vbe->vbe_mode = 0xFFFF;	
	mbi_vbe->vbe_interface_seg = 0;	
	mbi_vbe->vbe_interface_off = 0;	
	mbi_vbe->vbe_interface_len = 0;	
	VBE_StorePModeInfo((uintptr_t)(&(mbi_vbe->vbe_interface_seg)));

    struct VBE_Info* vbe_info = (struct VBE_Info*)(16 + (uintptr_t)mbi_vbe);
    uintptr_t vmodes = (uintptr_t)(vbe_info->video_modes);
    uint16_t* video_modes = (uint16_t*)(((vmodes >> 16) << 4) + (vmodes & 0xFFFF));

	uint16_t preferred_mode;
	if (width_requested == 0 && height_requested == 0 && depth_requested == 0) {
		if (EGA_text_supported) preferred_mode = VBE_GetTextMode(video_modes, 80, 25);
		else preferred_mode = VBE_GetCurrentMode();
	}
	else if (width_requested != 0 && height_requested != 0 && depth_requested == 0) {
		preferred_mode = VBE_GetTextMode(video_modes, width_requested, height_requested);
	}
	else preferred_mode = VBE_GetMode(video_modes, width_requested, height_requested, depth_requested);

	if (preferred_mode == 0xFFFF) preferred_mode = VBE_GetCurrentMode();
	if (preferred_mode == 0xFFFF || VBE_GetModeInfo(preferred_mode, (uintptr_t)mbi_vbe->vbe_mode_info) == false) {
		if (graphics_mbi_mandatory) return false;
		for (size_t i = 0; i < 0x100; i++) mbi_vbe->vbe_mode_info[i] = 0;
		return true;	
	}

	mbi_vbe->vbe_mode = preferred_mode;

	uintptr_t mem_fbuf_tag = Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
	if (mem_fbuf_tag == (uintptr_t)MEMORY_NULL_PTR) return false;
	struct Multiboot_Info_Framebuffer* mbi_fbuf = (struct Multiboot_Info_Framebuffer*)mem_fbuf_tag;
    if (mbi_fbuf->type != 0) return false;

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

	mbi_term = (struct Multiboot_Info_Tag*)(mbi_fbuf->size + (uintptr_t)mbi_fbuf);
	mbi_term->type = 0;
	mbi_term->size = 8;

	mbi_start->total_size += mbi_fbuf->size;

	if (!VBE_SetMode(preferred_mode)) return false;
	return true;
}

bool Multiboot_SaveELFSectionHeaders(uintptr_t mbi_addr, uintptr_t image) {

    if (!Multiboot_CheckForValidMBI(mbi_addr)) return false;
    struct Multiboot_Info_Start* mbi_start = (struct Multiboot_Info_Start*)mbi_addr;

    uintptr_t elf_shdr_tag = Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_ELF_SECTIONS);
    if (elf_shdr_tag == (uintptr_t)MEMORY_NULL_PTR) return false;
    struct Multiboot_Info_ELF_Sections* mbi_elf_shdr = (struct Multiboot_Info_ELF_Sections*)elf_shdr_tag;
    if (mbi_elf_shdr->type != 0) return false;

    mbi_elf_shdr->type    = MULTIBOOT_TAG_TYPE_ELF_SECTIONS;
	mbi_elf_shdr->num     = 0;
	mbi_elf_shdr->entsize = 0;
	mbi_elf_shdr->shndx   = 0;
    mbi_elf_shdr->size    = Elf32_LoadSectionHeaderTable(image, 8 + (uintptr_t)mbi_elf_shdr, true) + 8;

	struct Multiboot_Info_Tag* mbi_term = (struct Multiboot_Info_Tag*)(mbi_elf_shdr->size + (uintptr_t)mbi_elf_shdr);
	mbi_term->type = 0;
	mbi_term->size = 8;
	
	mbi_start->total_size += mbi_elf_shdr->size;

	return true;
}

bool Multiboot_CheckForSupportFailure(uintptr_t multiboot_header_ptr) {

    struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
    struct Multiboot_Header_Tag* header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);

    while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
        if ((header_tag->flags & 1) == 0 && header_tag->type == MULTIBOOT_HEADER_TAG_EFI_BS             ) return false;
        if ((header_tag->flags & 1) == 0 && header_tag->type == MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32) return false;
        if ((header_tag->flags & 1) == 0 && header_tag->type == MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64) return false;
        if ((header_tag->flags & 1) == 0 && header_tag->type == MULTIBOOT_HEADER_TAG_RELOCATABLE        ) return false;
        if ((header_tag->flags & 1) == 0 && header_tag->type == MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST) {
			struct Multiboot_Header_Tag_Information* header_requests = (struct Multiboot_Header_Tag_Information*)header_tag;
			size_t nrequests = (header_requests->size - 8)/sizeof(uint32_t);
			for (size_t i = 0; i < nrequests; i++) {
				if (header_requests->requests[i] == 1 || header_requests->requests[i] == 3 || header_requests->requests[i] == 5 || header_requests->requests[i] > 9) return false;
			}
		}
        header_tag = (struct Multiboot_Header_Tag*)((uintptr_t)header_tag + header_tag->size);
    }

	return true;
}

void PrintNum(uint32_t num, uint8_t line, uint8_t column) {

    line = line % 25;
    column = column % 80;

    uint32_t pos = 2 * (line * 80 + column);

    char* screen = (char*)0xB8000;

    screen[pos] = '0';
    pos++;
    screen[pos] = 0x0F;
    pos++;

    screen[pos] = 'x';
    pos++;
    screen[pos] = 0x0F;
    pos++;

    uint32_t divisor = 0x10000000;
    uint32_t digit = 0;

    for (size_t i = 0; i < 8; i++) {

        digit = num / divisor;

        uint8_t cdigit = (uint8_t)digit;

        if (digit < 0xA) cdigit += 0x30;
        else cdigit += (0x41 - 0xA);
        screen[pos] = cdigit;
        pos++;
        screen[pos] = 0x0F;
        pos++;

        num = num % divisor;
        divisor /= 0x10;

    }

}

