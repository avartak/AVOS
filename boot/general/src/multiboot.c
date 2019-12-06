#include <boot/general/include/multiboot.h>
#include <csupport/include/string.h>

extern size_t DiskIO_ReadFromDisk(uint8_t drive, uintptr_t mem_start_addr, uint64_t disk_start_sector, size_t num_sectors);

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
	if (header_magic->header_length < sizeof(struct Multiboot_Header_Magic_Fields)) return 0;
	if ((uint32_t)(header_magic->checksum + header_magic->magic + header_magic->architecture + header_magic->header_length) != 0) return 0;
	
	return multiboot_header_ptr;

}

bool Multiboot_LoadKernelFile(uintptr_t mbi_addr, struct Boot_Kernel_Info* kernel_info) {

	// Set up access to the memory map (so that we can find a good place to load the kernel file)
    struct Multiboot_Info_Tag* mbi_name = (struct Multiboot_Info_Tag*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_RAM_INFO);
    if (mbi_name == MEMORY_NULL_PTR || mbi_name->type == 0) return false;
    struct Boot_Block64* mmap = (struct Boot_Block64*)(0x10 + (uintptr_t)mbi_name);
    size_t mmap_size = (mbi_name->size - 0x10)/sizeof(struct Boot_Block64);

	bool file_loaded = false;

	// Load the kernel file in memory
	struct Boot_BlockList128* blocklist_mst = (struct Boot_BlockList128*)kernel_info->blocklist_ptr;
	uint8_t blocklist[0x1000];
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
					kernel_info->file_size = 0;
					for (size_t k = 0; k < BOOT_BLOCKLIST_MAXBLOCKS512 && blocklist512->blocks[k].num_sectors > 0; k++) {
						if (k == BOOT_BLOCKLIST_MAXBLOCKS512) return false;
						kernel_info->file_size += blocklist512->blocks[k].num_sectors * blocklist512->sector_size;
					}

					kernel_info->file_addr = Boot_MemoryBlockAboveAddress(BOOT_HIGH_MEMORY_START, kernel_info->file_size, 1, mmap, mmap_size);
					if (kernel_info->file_addr == BOOT_32BIT_MEMORY_LIMIT) return false;

					for (size_t k = 0; blocklist512->blocks[k].num_sectors > 0; k++) {
						uint64_t kern_lba = part_start + blocklist512->blocks[k].lba;
						if (DiskIO_ReadFromDisk((uint8_t)(kernel_info->boot_drive_ID), kernel_info->file_addr, kern_lba, blocklist512->blocks[k].num_sectors) != blocklist512->blocks[k].num_sectors * blocklist512->sector_size) return false;
					}
					file_loaded = true;
				}
			}
		}
	}

	// Save information of ELF section headers in the MBI (if requested)
	if (file_loaded) {
		uintptr_t multiboot_header_ptr = Multiboot_GetHeader(kernel_info->file_addr, kernel_info->file_size);
		if (multiboot_header_ptr == 0) return false;
		
		struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
		struct Multiboot_Header_Tag* header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);
		if (header_magic->architecture != MULTIBOOT_ARCHITECTURE_I386) return false;
		
		while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
			if (header_tag->type == MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST) {
				struct Multiboot_Header_Tag_Information* header_requests = (struct Multiboot_Header_Tag_Information*)header_tag;
				size_t nrequests = (header_requests->size - 8)/sizeof(uint32_t);
				for (size_t i = 0; i < nrequests; i++) {
					if (header_requests->requests[i] == MULTIBOOT_TAG_TYPE_ELF_SECTIONS) {
						struct Multiboot_Info_ELF_Sections* mbi_elf_shdr = (struct Multiboot_Info_ELF_Sections*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_ELF_SECTIONS);
						if ((mbi_elf_shdr == MEMORY_NULL_PTR || mbi_elf_shdr->type != 0) && (header_tag->flags & 1) == 0) return false;
						
						mbi_elf_shdr->type    = MULTIBOOT_TAG_TYPE_ELF_SECTIONS;
						mbi_elf_shdr->num     = 0;
						mbi_elf_shdr->entsize = 0;
						mbi_elf_shdr->shndx   = 0;
						mbi_elf_shdr->size    = Elf32_LoadSectionHeaderTable(kernel_info->file_addr, 8 + (uintptr_t)mbi_elf_shdr, true) + 8;
						
						Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_elf_shdr);
					}
				}
			}
			header_tag = (struct Multiboot_Header_Tag*)((uintptr_t)header_tag + header_tag->size);
		}
		return true;
	}

	return false;
}

	
bool Multiboot_LoadKernel(uintptr_t mbi_addr, struct Boot_Kernel_Info* kernel_info) {

    // Set up access to the memory map
    struct Multiboot_Info_Tag* mbi_name = (struct Multiboot_Info_Tag*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_RAM_INFO);
    if (mbi_name == MEMORY_NULL_PTR || mbi_name->type == 0) return false;
    struct Boot_Block64* mmap = (struct Boot_Block64*)(0x10 + (uintptr_t)mbi_name);
    size_t mmap_size = (mbi_name->size - 0x10)/sizeof(struct Boot_Block64);

	// Kernel load parameters
	uintptr_t file_addr    = kernel_info->file_addr;
	size_t    file_size    = kernel_info->file_size;
	
	bool      reloc        = false;
	uintptr_t start_min    = kernel_info->start;
	uintptr_t end_max      = kernel_info->start + file_size;
	uint32_t  load_pref    = 0;
	uint32_t  start_align  = 0x1000;
	
	bool      load_info    = false;
	uintptr_t load_start   = file_addr;

	uintptr_t kernel_entry = (uintptr_t)MEMORY_NULL_PTR;

	bool isELF             = Elf32_IsValidiStaticExecutable(file_addr);
	kernel_info->size      = isELF ? Elf32_StaticExecutableLoadSize(file_addr) : file_size;
	kernel_info->bss_size  = isELF ? Elf32_SizeBSSLikeSections(file_addr) : 0;
	
	// Set up kernel load parameters from the information in the multiboot header
	uintptr_t multiboot_header_ptr = Multiboot_GetHeader(file_addr, file_size);
	struct Multiboot_Header_Magic_Fields* header_magic = (struct Multiboot_Header_Magic_Fields*)multiboot_header_ptr;
	struct Multiboot_Header_Tag* header_tag = (struct Multiboot_Header_Tag*)(header_magic + 1);
	
	while ((uintptr_t)header_tag < multiboot_header_ptr + header_magic->header_length) {
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
			
			if (header_tag_address->load_addr == (uint32_t)(-1)) {
				load_start = file_addr;
				kernel_info->start = header_tag_address->header_addr - (multiboot_header_ptr - file_addr);
			}
			else {
				load_start = multiboot_header_ptr - (header_tag_address->header_addr - header_tag_address->load_addr);
				kernel_info->start = header_tag_address->load_addr;
			}

			if (header_tag_address->load_end_addr != 0 && header_tag_address->load_end_addr < kernel_info->start) return false; 
			kernel_info->size  = (header_tag_address->load_end_addr == 0  ? file_size : header_tag_address->load_end_addr - kernel_info->start);

			if (header_tag_address->bss_end_addr != 0 && header_tag_address->bss_end_addr < (kernel_info->start + kernel_info->size)) return false;		
			kernel_info->bss_size = (header_tag_address->bss_end_addr  == 0  ? 0 : header_tag_address->bss_end_addr - (kernel_info->start + kernel_info->size));
			kernel_info->size += kernel_info->bss_size;

			load_info  = true; 
		}
		if (header_tag->type == MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS) {
			struct Multiboot_Header_Tag_Entry_Address* header_tag_entry = (struct Multiboot_Header_Tag_Entry_Address*)header_tag;
			kernel_entry = header_tag_entry->entry_addr;
			break;
		}
		header_tag = (struct Multiboot_Header_Tag*)((uintptr_t)header_tag + header_tag->size);
	}
	
	if (kernel_info->size == 0) return false;
	if (reloc && end_max - start_min < kernel_info->size) return false;

	// Adjust the start address of the kernel if possible/requested/needed
	if (!load_info && reloc) {
		uint32_t kstart = kernel_info->start;
		if (load_pref == 0 || load_pref == 1) {
			kstart = Boot_MemoryBlockAboveAddress(start_min, kernel_info->file_size, start_align, mmap, mmap_size);
			if (kstart == BOOT_32BIT_MEMORY_LIMIT || kstart + kernel_info->size > end_max) return false;
		}
		else if (load_pref == 2) {
			kstart = Boot_MemoryBlockBelowAddress(  end_max, kernel_info->file_size, start_align, mmap, mmap_size);
			if (kstart == BOOT_32BIT_MEMORY_LIMIT || kstart < start_min) return false;
		}
		kernel_info->start = kstart;
	}
	
	// Load the kernel from the kernel file in memory
	if (Boot_MemoryBlockAboveAddress(kernel_info->start, kernel_info->file_size, 1, mmap, mmap_size) != kernel_info->start) return false;
	if (load_info) {
		memmove((uint8_t*)(kernel_info->start), (uint8_t*)load_start, kernel_info->size - kernel_info->bss_size);
		memset ((uint8_t*)(kernel_info->start + kernel_info->size - kernel_info->bss_size), 0, kernel_info->bss_size);
	}
	else if (Elf32_IsValidiStaticExecutable(file_addr)) Elf32_LoadStaticExecutable(file_addr, kernel_info->start);
	else memmove((uint8_t*)(kernel_info->start), (uint8_t*)load_start, kernel_info->size);
	
	// Clean up traces of the kernel file image in memory
	uintptr_t end_addr = kernel_info->start + kernel_info->size;
	uintptr_t cleanup_start = (end_addr > file_addr ? end_addr : file_addr);
	if (cleanup_start < file_addr + file_size) memset((uint8_t*)cleanup_start, 0, file_addr + file_size - cleanup_start);
	if (file_addr < kernel_info->start) memset((uint8_t*)file_addr, 0, (kernel_info->start - file_addr) > file_size ? file_size : (kernel_info->start - file_addr));
	
	// Update the kernel information structure (will be needed/used by other functions subsequently)
	kernel_info->entry = (kernel_entry == (uintptr_t)MEMORY_NULL_PTR ? kernel_info->start : kernel_entry);
	kernel_info->multiboot_header = Multiboot_GetHeader(kernel_info->start, kernel_info->size);

	return true;
	
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
	
    struct Multiboot_Info_Tag* mbi_name = (struct Multiboot_Info_Tag*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_RAM_INFO);
    if (mbi_name == MEMORY_NULL_PTR || mbi_name->type == 0) return false;
    struct Boot_Block64* mmap = (struct Boot_Block64*)(0x10 + (uintptr_t)mbi_name);
    size_t mmap_size = (mbi_name->size - 0x10)/sizeof(struct Boot_Block64);

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
				for (size_t k = 0; k < BOOT_BLOCKLIST_MAXBLOCKS272 && blocklist272->blocks[k].num_sectors > 0; k++) mod_size += blocklist272->blocks[k].num_sectors*blocklist272->sector_size;
				if (mod_size == 0) continue;
				mod_addr = Boot_MemoryBlockAboveAddress(kernel_info->start, kernel_info->file_size, (page_align ? 0x1000 : 1), mmap, mmap_size);
				if (mod_addr == BOOT_32BIT_MEMORY_LIMIT) return false;
				for (size_t k = 0; k < BOOT_BLOCKLIST_MAXBLOCKS272 && blocklist272->blocks[k].num_sectors > 0; k++) {
					uint64_t mod_lba = part_start + blocklist272->blocks[k].lba;
					bytes_read = DiskIO_ReadFromDisk((uint8_t)(kernel_info->boot_drive_ID), mod_addr, mod_lba, blocklist272->blocks[k].num_sectors);
					if (bytes_read != blocklist272->blocks[k].num_sectors*blocklist272->sector_size) return false;
				}

				struct Multiboot_Info_Modules* mbi_mod = (struct Multiboot_Info_Modules*)Multiboot_FindMBITagAddress(mbi_addr, MULTIBOOT_TAG_TYPE_MODULE);
				if (mbi_mod == MEMORY_NULL_PTR || mbi_mod->type != 0) return false;
				mbi_mod->type = MULTIBOOT_TAG_TYPE_MODULE;
				mbi_mod->size = 256;
				mbi_mod->mod_start = mod_addr;
				mbi_mod->mod_end = mod_addr + mod_size - 1;
				for (size_t k = 0; i < 240; k++) mbi_mod->string[k] = blocklist272->string[k];
				if (!Multiboot_TerminateTag(mbi_addr, (uintptr_t)mbi_mod)) return false;
				mod_addr += mod_size;
			}
		}
	}
	return true;	
}

