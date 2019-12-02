#include <boot/x86/BIOS/include/multiboot.h>
#include <boot/x86/BIOS/include/diskio.h>
#include <boot/x86/BIOS/include/RAM.h>
#include <boot/x86/BIOS/include/VBE.h>
#include <boot/x86/BIOS/include/console.h>
#include <boot/x86/BIOS/include/discovery.h>
#include <boot/general/include/ELF.h>

bool Multiboot_CheckForValidMBI(uintptr_t mbi_addr) {

	struct Multiboot_Info_Start* mbi_start = (struct Multiboot_Info_Start*)mbi_addr;
	if ((mbi_addr % 8) != 0 || mbi_start->reserved != 0 || mbi_start->total_size < 16 || (mbi_start->total_size % 8) != 0) return false;
	
	struct Multiboot_Info_Tag* mbi_term_tag = (struct Multiboot_Info_Tag*)(mbi_start->total_size + mbi_addr - 8);
	if (mbi_term_tag->type != 0 || mbi_term_tag->size != 8) return false;
	else return true;
}

bool Multiboot_CreateEmptyMBI(uintptr_t mbi_addr) {

	if (mbi_addr % 8 != 0) return false;
	
	struct Multiboot_Info_Start* mbi_start = (struct Multiboot_Info_Start*)mbi_addr;
	mbi_start->total_size = 16;
	mbi_start->reserved   = 0;
	
	struct Multiboot_Info_Tag* mbi_term_tag = (struct Multiboot_Info_Tag*)(mbi_addr + 8);
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

bool Multiboot_TerminateTag(uintptr_t mbi_addr, uintptr_t tag_addr) {

	struct Multiboot_Info_Start* mbi_start = (struct Multiboot_Info_Start*)mbi_addr;
	if ((mbi_addr % 8) != 0 || mbi_start->reserved != 0 || (mbi_start->total_size % 8) != 0) return false;
	
	struct Multiboot_Info_Tag* mbi_tag  = (struct Multiboot_Info_Tag*)tag_addr;
	struct Multiboot_Info_Tag* mbi_term = (struct Multiboot_Info_Tag*)(mbi_tag->size + tag_addr);
	
	if (mbi_tag->type == 0 && mbi_tag->size == 8) return true;
	if (mbi_tag->size == 0) {
		mbi_term->type = 0;
		mbi_term->size = 8;
	}
	else {
		mbi_term->type = 0;
		mbi_term->size = 8;
		mbi_start->total_size += mbi_tag->size;
	}
	return true;

}

uintptr_t Multiboot_GetHeader(uintptr_t start_addr, size_t size) {

	uint32_t* start_ptr_u32 = (uint32_t*)start_addr;
	
	uintptr_t multiboot_header_ptr = start_addr;
	bool found_header_magic = false;
	for (size_t i = 0; i < size/4 && i < 0x2000; i++) {
		if (i % 2 != 0) continue;
		if (start_ptr_u32[i] == 0xE85250D6) {
			multiboot_header_ptr = (uintptr_t)(start_ptr_u32 + i);
			found_header_magic = true;
			break;
		}
	}
	
	if (!found_header_magic) return 0;
	
	struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
	if (header_magic->architecture != MULTIBOOT_ARCHITECTURE_I386) return 0;
	if (header_magic->header_length < sizeof(struct Multiboot_Header_Magic_Fields)) return 0;
	if ((uint32_t)(header_magic->checksum + header_magic->magic + header_magic->architecture + header_magic->header_length) != 0) return 0;
	
	return multiboot_header_ptr;

}

uintptr_t Multiboot_GetKernelEntry(uintptr_t multiboot_header_ptr) {
   
	struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
	struct Multiboot_Header_Tag* header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);
	
	uintptr_t kernel_entry = (uintptr_t)MEMORY_NULL_PTR;
	
	while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
		if (header_tag->type == MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS) {
			struct Multiboot_Header_Tag_Entry_Address* header_tag_entry = (struct Multiboot_Header_Tag_Entry_Address*)header_tag;
			kernel_entry = header_tag_entry->entry_addr;
			break;
		}
		header_tag = (struct Multiboot_Header_Tag*)((uintptr_t)header_tag + header_tag->size);
	}
	
	return kernel_entry;
}


bool Multiboot_LoadKernel(uintptr_t mbi_addr, struct Boot_Kernel_Info* kernel_info) {

	uintptr_t image         = 0;
	size_t    file_size     = 0;
	
	struct Boot_BlockList128* blocklist_mst = (struct Boot_BlockList128*)kernel_info->blocklist_ptr;
	uint8_t blocklist[0x1000];
	bool kernel_found = false;
	for (size_t i = 0; i < BOOT_BLOCKLIST_MAXBLOCKS128 && blocklist_mst->blocks[i].num_sectors > 0; i++) {
		for (size_t j = 0; j < blocklist_mst->blocks[i].num_sectors; j++) {
			uint64_t part_start = ((uint64_t*)(kernel_info->part_info_ptr))[0];
			uint64_t offset = blocklist_mst->blocks[i].lba + j;
			uint64_t lba = part_start + offset;
			
			size_t bytes_read = DiskIO_ReadFromDisk((uint8_t)(kernel_info->boot_drive_ID), (uintptr_t)blocklist, lba, 1); 
			if (bytes_read == 0 || bytes_read != blocklist_mst->sector_size || bytes_read > 0x1000) return false;
			
			struct Boot_BlockList512* blocklist512 = (struct Boot_BlockList512*)(blocklist) - 1;
			for (size_t s = 0; s < blocklist_mst->sector_size/0x200; s++) {
				blocklist512++;
				if (blocklist512->jump != 0x9001FDE9) continue;
				char* marker = blocklist512->reserved;
				if (marker[0] == 'K' && marker[1] == 'E' && marker[2] == 'R' && marker[3] == 'N' && marker[4] == 'E' && marker[5] == 'L') {
					image = blocklist512->load_address_lo;
					kernel_found = true;
					for (size_t k = 0; k < BOOT_BLOCKLIST_MAXBLOCKS512 && blocklist512->blocks[k].num_sectors > 0; k++) {
						uint64_t kern_lba = part_start + blocklist512->blocks[k].lba;
						bytes_read = DiskIO_ReadFromDisk((uint8_t)(kernel_info->boot_drive_ID), image, kern_lba, blocklist512->blocks[k].num_sectors);
						if (bytes_read != blocklist512->blocks[k].num_sectors * blocklist512->sector_size) return false;
						file_size += bytes_read;
					}
				}
				if (kernel_found) break;
			}
			if (kernel_found) break;
		}
		if (kernel_found) break;
	}
	if (!kernel_found) return false;


	uintptr_t start_addr    = kernel_info->start;
	bool      reloc         = false;
	uintptr_t start_min     = start_addr;
	uintptr_t end_max       = start_addr + file_size;
	uint32_t  load_pref     = 0;
	uint32_t  start_align   = 0x1000;
	
	bool      kernel_loaded = false;
	bool      load_info     = false;
	uintptr_t load_addr     = start_addr;
	uintptr_t load_start    = image;
	size_t    load_size     = file_size;
	size_t    bss_size      = 0;
	
	uintptr_t multiboot_header_ptr = Multiboot_GetHeader(image, file_size);
	if (multiboot_header_ptr == 0) return false;
	
	struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
	struct Multiboot_Header_Tag* header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);
	
	while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
		if (header_tag->type == MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST) {
			struct Multiboot_Header_Tag_Information* header_requests = (struct Multiboot_Header_Tag_Information*)header_tag;
			size_t nrequests = (header_requests->size - 8)/sizeof(uint32_t);
			for (size_t i = 0; i < nrequests; i++) {
				if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_ELF_SECTIONS) {
					if (!Multiboot_SaveELFSectionHeaders(mbi_addr, image)) return false;
				}
			}
		}
		if (header_tag->type == MULTIBOOT_HEADER_TAG_RELOCATABLE) {
			struct Multiboot_Header_Tag_Relocatable* header_reloc = (struct Multiboot_Header_Tag_Relocatable*)header_tag;
			reloc       = true;
			start_min   = header_reloc->min_addr;
			end_max     = header_reloc->max_addr;
			load_pref   = header_reloc->preference;
			start_align = header_reloc->align;
		}
		if (header_tag->type == MULTIBOOT_HEADER_TAG_ADDRESS) {
			struct Multiboot_Header_Tag_Address* header_tag_address = (struct Multiboot_Header_Tag_Address*)header_tag;
			if (header_tag_address->load_addr > header_tag_address->header_addr) return false;
			
			load_addr  = header_tag_address->load_addr;	
			load_start = (header_tag_address->load_addr     == (uint32_t)(-1) ? image     : multiboot_header_ptr - (header_tag_address->header_addr - header_tag_address->load_addr));
			load_size  = (header_tag_address->load_end_addr ==             0  ? file_size : header_tag_address->load_end_addr - header_tag_address->load_addr);
			bss_size   = (header_tag_address->bss_end_addr  ==             0  ? 0         : header_tag_address->bss_end_addr - header_tag_address->load_end_addr);
			load_info  = true; 
		}
		header_tag = (struct Multiboot_Header_Tag*)((uintptr_t)header_tag + header_tag->size);
	}
	
	if (!load_info && Elf32_IsValidiStaticExecutable(image)) load_size = Elf32_StaticExecutableLoadSize(image);
	if (load_size == 0) return false;
	if (reloc && end_max - start_min < load_size + bss_size) return false;
	if (reloc && load_pref == 0) {
		if (start_addr < start_min) {
			start_addr = start_min;
			if (start_align > 1 && start_addr % start_align != 0) start_addr += start_align - start_addr % start_align;
			if (start_addr + load_size + bss_size > end_max) return false;
		}
		else if (start_addr + load_size + bss_size > end_max) {
			start_addr = end_max - load_size - bss_size;
			if (start_align > 1 && start_addr % start_align != 0) start_addr -= start_addr % start_align;
			if (start_addr < start_min) return false;
		}
	}		
	else if (reloc && load_pref == 1) {
		start_addr = start_min;
		if (start_align > 1 && start_addr % start_align != 0) start_addr += start_align - start_addr % start_align;
		if (start_addr + load_size + bss_size > end_max) return false;
	}
	else if (reloc && load_pref == 2) {
		start_addr = end_max - load_size - bss_size;
		if (start_align > 1 && start_addr % start_align != 0) start_addr -= start_addr % start_align;
		if (start_addr < start_min) return false;
	}
	
	if (load_addr != (uint32_t)(-1) && load_addr != start_addr) {
		kernel_info->start = load_addr;
		start_addr = load_addr;
		if (reloc) {
			if (load_addr < start_min || load_addr + load_size + bss_size > end_max || (start_align > 1 && load_addr % start_align != 0)) return false;
		}
	}
	
	
	header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);
	while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
		if (header_tag->type == MULTIBOOT_HEADER_TAG_ADDRESS) {
			struct Multiboot_Header_Tag_Address* header_tag_address = (struct Multiboot_Header_Tag_Address*)header_tag;
			if (header_tag_address->load_addr > header_tag_address->header_addr) return false;
			
			uintptr_t load_header_addr = header_tag_address->header_addr;
			
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
			if (load_size > 0 && (!reloc || start_addr + load_size <= end_max)) kernel_loaded = true;
		}
		else {
			memmove((uint8_t*)start_addr, (uint8_t*)load_start, load_size);
			if (load_size > 0 && (!reloc || start_addr + load_size <= end_max)) kernel_loaded = true;
		}
	}
	
	if (kernel_loaded) {
		uintptr_t end_addr = start_addr + load_size + bss_size;
		uintptr_t image_cleanup_start = (end_addr > image ? end_addr : image);
		if (image_cleanup_start < image + file_size) memset((uint8_t*)image_cleanup_start, 0, image + file_size - image_cleanup_start);
		
		kernel_info->size = load_size + bss_size;
		kernel_info->multiboot_header = Multiboot_GetHeader(start_addr, kernel_info->size);
		if (kernel_info->multiboot_header == 0) return false;
		uintptr_t entry = Multiboot_GetKernelEntry(kernel_info->multiboot_header);
		kernel_info->entry = (entry == (uintptr_t)MEMORY_NULL_PTR ? start_addr : entry);
		return true;
	}
	else return false;
	
}

bool Multiboot_LoadModules(uintptr_t mbi_addr, struct Boot_Kernel_Info* kernel_info) {

	bool page_align = false;
	
	uintptr_t multiboot_header_ptr = kernel_info->multiboot_header;
	struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
	struct Multiboot_Header_Tag* header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);
	
	while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
	    if (header_tag->type == MULTIBOOT_HEADER_TAG_MODULE_ALIGN) page_align = true;
	    header_tag = (struct Multiboot_Header_Tag*)((uintptr_t)header_tag + header_tag->size);
	}
	
	struct Boot_BlockList128* blocklist_mst = (struct Boot_BlockList128*)kernel_info->blocklist_ptr;
	uint8_t blocklist[0x1000];
	uintptr_t mod_addr = kernel_info->start + kernel_info->size;
	for (size_t i = 0; i < BOOT_BLOCKLIST_MAXBLOCKS128 && blocklist_mst->blocks[i].num_sectors > 0; i++) {
		for (size_t j = 0; j < blocklist_mst->blocks[i].num_sectors; j++) {
			uint64_t part_start = ((uint64_t*)(kernel_info->part_info_ptr))[0];
			uint64_t offset = blocklist_mst->blocks[i].lba + j;
			uint64_t lba = part_start + offset;
			
			size_t bytes_read = DiskIO_ReadFromDisk((uint8_t)(kernel_info->boot_drive_ID), (uintptr_t)blocklist, lba, 1);
			if (bytes_read == 0 || bytes_read != blocklist_mst->sector_size || bytes_read > 0x1000) return false;
			
			struct Boot_BlockList512* blocklist512 = (struct Boot_BlockList512*)blocklist - 1;
			for (size_t s = 0; s < blocklist_mst->sector_size/0x200; s++) {
				blocklist512++;
				if (blocklist512->jump != 0x9001FDE9) continue;
				char* marker = blocklist512->reserved;
				if (marker[0] == 'K' && marker[1] == 'E' && marker[2] == 'R' && marker[3] == 'N' && marker[4] == 'E' && marker[5] == 'L') continue;
				
				struct Boot_BlockList272* blocklist272 = (struct Boot_BlockList272*)blocklist512;
				size_t mod_size = 0;
				if (page_align && mod_addr % 0x1000 != 0) mod_addr = 0x1000 * (1 + mod_addr / 0x1000);
				for (size_t k = 0; k < BOOT_BLOCKLIST_MAXBLOCKS272 && blocklist272->blocks[k].num_sectors > 0; k++) {
					uint64_t mod_lba = part_start + blocklist272->blocks[k].lba;
					bytes_read = DiskIO_ReadFromDisk((uint8_t)(kernel_info->boot_drive_ID), mod_addr, mod_lba, blocklist272->blocks[k].num_sectors);
					if (bytes_read != blocklist272->blocks[k].num_sectors*blocklist272->sector_size) return false;
					mod_size += bytes_read;
				}
				if (mod_size > 0) {
					struct Multiboot_Info_Modules* mbi_mod = (struct Multiboot_Info_Modules*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_MODULE);
					if (mbi_mod == MEMORY_NULL_PTR || mbi_mod->type != 0) return false;
					mbi_mod->type = MULTIBOOT_TAG_TYPE_MODULE;
					mbi_mod->size = 256;
					mbi_mod->mod_start = mod_addr;
					mbi_mod->mod_end = mod_addr + mod_size - 1;
					for (size_t k = 0; i < 240; k++) mbi_mod->string[k] = blocklist272->string[k];
					if (!Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_mod)) return false;
				}
				mod_addr += mod_size;
			}
		}
	}
	return true;	
}

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

	struct Multiboot_Info_Memory_E820* mbi_mem_e820 = (struct Multiboot_Info_Memory_E820*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_MMAP);
	if (mbi_mem_e820 == MEMORY_NULL_PTR || mbi_mem_e820->type != 0) return false;
	
	mbi_mem_e820->size = RAM_StoreE820Info(16 + (uintptr_t)mbi_mem_e820) - (uintptr_t)mbi_mem_e820;
	mbi_mem_e820->type = MULTIBOOT_TAG_TYPE_MMAP;
	mbi_mem_e820->entry_size = 24;
	mbi_mem_e820->entry_version = 0;
	
	Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_mem_e820);
	
	if (mbi_mem_e820->size == 16) return false;
	
	uintptr_t mem_ram_tag = Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_RAM_INFO);
	if (mem_ram_tag == (uintptr_t)MEMORY_NULL_PTR) return false;
	struct Multiboot_Info_Memory_E820* mbi_mem_ram = (struct Multiboot_Info_Memory_E820*)mem_ram_tag;
	if (mbi_mem_ram->type != 0) return false;
	
	mbi_mem_ram->size = RAM_StoreInfo(16 + (uintptr_t)mbi_mem_ram, (struct Multiboot_E820_Entry*)(16 + (uintptr_t)mbi_mem_e820), (mbi_mem_e820->size - 16)/sizeof(struct Multiboot_E820_Entry)) - (uintptr_t)mbi_mem_ram;
	mbi_mem_ram->type = MULTIBOOT_TAG_TYPE_RAM_INFO;
	mbi_mem_ram->entry_size = sizeof(struct Boot_Block64);
	mbi_mem_ram->entry_version = 0;
	
	Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_mem_ram);
	
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

bool Multiboot_SaveELFSectionHeaders(uintptr_t mbi_addr, uintptr_t image) {

	struct Multiboot_Info_ELF_Sections* mbi_elf_shdr = (struct Multiboot_Info_ELF_Sections*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_ELF_SECTIONS);
	if (mbi_elf_shdr == MEMORY_NULL_PTR || mbi_elf_shdr->type != 0) return false;
	
	mbi_elf_shdr->type    = MULTIBOOT_TAG_TYPE_ELF_SECTIONS;
	mbi_elf_shdr->num     = 0;
	mbi_elf_shdr->entsize = 0;
	mbi_elf_shdr->shndx   = 0;
	mbi_elf_shdr->size    = Elf32_LoadSectionHeaderTable(image, 8 + (uintptr_t)mbi_elf_shdr, true) + 8;
	
	Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_elf_shdr);
	
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

bool Multiboot_Boot(uintptr_t mbi_addr, struct Boot_Kernel_Info* kernel_info) {

	Console_PrintBanner();
	
	if (!Multiboot_CreateEmptyMBI(mbi_addr)) return Console_PrintError("Error creating multiboot information record", false);
	
	if (!Multiboot_SaveMemoryMaps(mbi_addr)) return Console_PrintError("Error saving memory maps", false);
	
	struct Multiboot_Info_Memory_E820* mbi_name = (struct Multiboot_Info_Memory_E820*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_MMAP);
	if (mbi_name == MEMORY_NULL_PTR || mbi_name->type == 0) return Console_PrintError("Unable to detect memory map", false);
	struct Multiboot_E820_Entry* E820_Table = (struct Multiboot_E820_Entry*)(0x10 + (uintptr_t)mbi_name);
	size_t E820_Table_size = (mbi_name->size - 0x10)/sizeof(struct Multiboot_E820_Entry);
	if (!RAM_IsMemoryPresent(0x100000, 0xC00000, E820_Table, E820_Table_size)) return Console_PrintError("Insufficient memory to load OS", false);
	
	if (!Multiboot_LoadKernel (mbi_addr, kernel_info)) return Console_PrintError("Unable to load OS kernel", false); 
	if (!Multiboot_LoadModules(mbi_addr, kernel_info)) return Console_PrintError("Unable to load OS modules", false); 
	if (!Multiboot_SaveInfo   (mbi_addr, kernel_info)) return Console_PrintError("Unable to load OS boot information", false);
	
	return true;
}
