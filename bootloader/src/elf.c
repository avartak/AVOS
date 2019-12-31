#include <bootloader/include/elf.h>
#include <csupport/include/string.h>

bool Elf32_IsValidELF(uint32_t image) {

	Elf32_Ehdr* hdr = (Elf32_Ehdr*)image;
	
	if (hdr->e_ident[EI_MAG0] != ELFMAG0 || hdr->e_ident[EI_MAG1] != ELFMAG1 || hdr->e_ident[EI_MAG2] != ELFMAG2 || hdr->e_ident[EI_MAG3] != ELFMAG3) return false;
	if (hdr->e_ident[EI_CLASS] != ELFCLASS32) return false;
	if (hdr->e_ident[EI_DATA] != ELFDATA2LSB) return false;
	if (hdr->e_ident[EI_VERSION] != EV_CURRENT || hdr->e_version != EV_CURRENT) return false;
	if (hdr->e_machine != EM_386) return false;
	if (hdr->e_ehsize != sizeof(Elf32_Ehdr)) return false;
	
	return true;
}

bool Elf32_IsValidExecutable(uint32_t image) {

	Elf32_Ehdr* hdr = (Elf32_Ehdr*)image;
	
	if (!Elf32_IsValidELF(image)) return false;
	if (hdr->e_type != ET_EXEC) return false;
	if (hdr->e_phentsize != sizeof(Elf32_Phdr)) return false;
	if (hdr->e_phnum == 0) return false;
	
	return true;
}

bool Elf32_IsValidiStaticExecutable(uint32_t image) {

	Elf32_Ehdr* hdr = (Elf32_Ehdr*)image;
	
	if (!Elf32_IsValidExecutable(image)) return false;
	
	Elf32_Phdr* phdr = (Elf32_Phdr*)(image + hdr->e_phoff);
	
	for (uint32_t i = 0; i < hdr->e_phnum; i++) {
		if (phdr[i].p_type == PT_DYNAMIC || phdr[i].p_type == PT_INTERP) return false;
	}
	
	return true;
}

uint32_t Elf32_StaticExecutableLoadSize(uint32_t image) {

	uint32_t load_size = 0;
	
	if (!Elf32_IsValidiStaticExecutable(image)) return load_size;
	
	Elf32_Ehdr*  hdr = (Elf32_Ehdr*)image;
	Elf32_Phdr* phdr = (Elf32_Phdr*)(image + hdr->e_phoff);
	
	for (uint32_t i = 0; i < hdr->e_phnum; i++) {
		if (phdr[i].p_type == PT_LOAD) load_size += phdr[i].p_memsz;
	}
	
	return load_size;

}


uint32_t Elf32_LoadStaticExecutable(uint32_t image, uint32_t start_addr) {

	uint32_t load_size = 0;
	
	if (!Elf32_IsValidiStaticExecutable(image)) return load_size;
	
	Elf32_Ehdr*  hdr = (Elf32_Ehdr*)image;
	Elf32_Phdr* phdr = (Elf32_Phdr*)(image + hdr->e_phoff);

	uint8_t* p_mem_addr = (uint8_t*)start_addr;
	for (uint32_t i = 0; i < hdr->e_phnum; i++) {
		if (phdr[i].p_type != PT_LOAD) continue;
		
		uint8_t* p_img_addr = (uint8_t*)(image + phdr[i].p_offset);
		uint32_t p_img_size = phdr[i].p_filesz;
		uint32_t p_pad_size = phdr[i].p_memsz - phdr[i].p_filesz;
		
		memmove(p_mem_addr, p_img_addr, p_img_size);
		p_mem_addr += p_img_size;
		memset(p_mem_addr, 0, p_pad_size);
		p_mem_addr += p_pad_size;
		load_size += p_img_size + p_pad_size;
	}
	
	return load_size;

}

uint32_t Elf32_LoadSectionHeaderTable(uint32_t image, uint32_t start_addr, bool load_extra) {

	Elf32_Ehdr*  hdr = (Elf32_Ehdr*)image;
	Elf32_Shdr *shdr = (Elf32_Shdr*)(image + hdr->e_shoff);

	uint32_t retval = 0;
	if (load_extra) retval = 8;
	if (Elf32_IsValidELF(image)) {
		if (load_extra) {
			uint16_t* shinfo = (uint16_t*)start_addr;
			shinfo[0] = hdr->e_shnum;
			shinfo[1] = hdr->e_shentsize;
			shinfo[2] = hdr->e_shstrndx;
			shinfo[3] = 0;
		}
		memmove((uint8_t*)(retval+start_addr), (uint8_t*)shdr, hdr->e_shnum * hdr->e_shentsize);
		retval += hdr->e_shnum * hdr->e_shentsize;
	}	

	return retval;
}

uint32_t Elf32_SizeBSSLikeSections(uint32_t image) {

	if (!Elf32_IsValidELF(image)) return 0;
	
	Elf32_Ehdr*  hdr = (Elf32_Ehdr*)image;
	Elf32_Shdr* shdr = (Elf32_Shdr*)(image + hdr->e_shoff);
	Elf32_Phdr* phdr = (Elf32_Phdr*)(image + hdr->e_phoff);
	
	uint32_t bss_size = 0;
	
	if (hdr->e_shnum > 0) {
		for (uint32_t i = 0; i < hdr->e_shnum; i++) {
			if(shdr[i].sh_type == SHT_NOBITS && (shdr[i].sh_flags & SHF_ALLOC) && shdr[i].sh_size > 0) bss_size += shdr[i].sh_size;
		}
	}
	else {
		for (uint32_t i = 0; i < hdr->e_phnum; i++) {
			if (phdr[i].p_type != PT_LOAD) continue;
			if (phdr[i].p_filesz != 0) continue;
			bss_size += phdr[i].p_memsz;	
		}
	}
	
	return bss_size;

}

uint32_t Elf32_LoadBSSLikeSections(uint32_t image, uint32_t start_addr) {

	if (!Elf32_IsValidELF(image)) return 0;
	
	Elf32_Ehdr*  hdr = (Elf32_Ehdr*)image;
	Elf32_Shdr *shdr = (Elf32_Shdr*)(image + hdr->e_shoff);
	
	uint32_t bss_size = 0;
	uint8_t* mem = (uint8_t*)start_addr;
	for (uint32_t i = 0; i < hdr->e_shnum; i++) {
		if(shdr[i].sh_type == SHT_NOBITS && (shdr[i].sh_flags & SHF_ALLOC) && shdr[i].sh_size > 0) {
			memset(mem, 0, shdr[i].sh_size);
			mem      += shdr[i].sh_size;
			bss_size += shdr[i].sh_size;
		}
	}
	
	return bss_size;

}

