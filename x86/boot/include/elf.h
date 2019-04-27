#ifndef X86_BOOT_ELF_H
#define X86_BOOT_ELF_H

#include <kernel/include/common.h>

typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;

/* Size if the ELF32 idenitifer 8-byte array */

#define ELF_NIDENT      16

/* ELF header */
 
typedef struct {
	uint8_t		e_ident[ELF_NIDENT];
	Elf32_Half	e_type;
	Elf32_Half	e_machine;
	Elf32_Word	e_version;
	Elf32_Addr	e_entry;
	Elf32_Off	e_phoff;
	Elf32_Off	e_shoff;
	Elf32_Word	e_flags;
	Elf32_Half	e_ehsize;
	Elf32_Half	e_phentsize;
	Elf32_Half	e_phnum;
	Elf32_Half	e_shentsize;
	Elf32_Half	e_shnum;
	Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

/* Following values can be used in the e_type field of the ELF header */

#define ET_NONE          0
#define ET_REL           1
#define ET_EXEC          2
#define ET_DYN           3
#define ET_CORE          4
#define ET_LOPROC        0xFF00
#define ET_HIPROC        0xFFFF

/* Indexes for certain contents of the ELF32 identifier */

#define EI_MAG0          0
#define EI_MAG1          1
#define EI_MAG2          2
#define EI_MAG3          3
#define EI_CLASS         4
#define EI_DATA          5
#define EI_VERSION       6
#define EI_OSABI         7
#define EI_ABIVERSION    8
#define EI_PAD           9
#define EI_NIDENT       16    // ---> This is simply the size of e_ident[], not an index

/* Magic fields (first four bytes) in the e_ident[] array of the ELF header */

#define ELFMAG0          0x7F
#define ELFMAG1          'E'
#define ELFMAG2          'L'
#define ELFMAG3          'F'

/* The EI_CLASS byte in the e_ident[] array of the ELF header*/

#define ELFCLASSNONE     0
#define ELFCLASS32       1
#define ELFCLASS64       2

/* The EI_DATA byte in the e_ident[] array of the ELF header*/

#define ELFDATANONE      0
#define ELFDATA2LSB      1
#define ELFDATA2MSB      2

/* Machine types that can be used in e_machine field of the ELF header */

#define EM_NONE          0
#define EM_M32           1
#define EM_SPARC         2
#define EM_386           3
#define EM_68K           4
#define EM_88K           5
#define EM_860           7
#define EM_MIPS          8
#define EM_MIPS_RS4_BE  10

/* ELF version information to be used in the e_version field of the ELF header */

#define EV_NONE          0
#define EV_CURRENT       1

/* Program header */

typedef struct {
	Elf32_Word		p_type;
	Elf32_Off		p_offset;
	Elf32_Addr		p_vaddr;
	Elf32_Addr		p_paddr;
	Elf32_Word		p_filesz;
	Elf32_Word		p_memsz;
	Elf32_Word		p_flags;
	Elf32_Word		p_align;
} Elf32_Phdr;

/* Type of the program segment, to be used in the p_type field of the program header */

#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7FFFFFFF

/* Bit field values to be used in the p_flags field of the program header */

#define PF_X         1
#define PF_W         2
#define PF_R         4
#define PF_MASKPROC  0xF0000000

/* Section header  */

typedef struct {
	Elf32_Word	sh_name;
	Elf32_Word	sh_type;
	Elf32_Word	sh_flags;
	Elf32_Addr	sh_addr;
	Elf32_Off	sh_offset;
	Elf32_Word	sh_size;
	Elf32_Word	sh_link;
	Elf32_Word	sh_info;
	Elf32_Word	sh_addralign;
	Elf32_Word	sh_entsize;
} Elf32_Shdr;

/* Special section indexes */

#define SHN_UNDEF     0
#define SHN_LORESERVE 0xFF00
#define SHN_LOPROC    0xFF00
#define SHN_HIPROC    0xFF1F
#define SHN_ABS       0xFFF1
#define SHN_COMMON    0xFFF2
#define SHN_HIRESERVE 0xFFFF

/* Section type to be used in the sh_type field of the section header */

#define SHT_NULL      0
#define SHT_PROGBITS  1
#define SHT_SYMTAB    2
#define SHT_STRTAB    3
#define SHT_RELA      4
#define SHT_HASH      5
#define SHT_DYNAMIC   6
#define SHT_NOTE      7
#define SHT_NOBITS    8
#define SHT_REL       9
#define SHT_SHLIB    10 
#define SHT_DYNSYM   11
#define SHT_LOPROC   0x70000000
#define SHT_HIPROC   0x7FFFFFFF
#define SHT_LOUSER   0x80000000
#define SHT_HIUSER   0xFFFFFFFF

/* Bit field values to be used in the sh_flags field of the section header */

#define SHF_WRITE     1
#define SHF_ALLOC     2
#define SHF_EXECINSTR 4
#define SHF_MASKPROC  0xF0000000

/* Symbol table */

typedef struct {
	Elf32_Word		st_name;
	Elf32_Addr		st_value;
	Elf32_Word		st_size;
	uint8_t			st_info;
	uint8_t			st_other;
	Elf32_Half		st_shndx;
} Elf32_Sym;

/* Undefined symbol table index */

#define STN_UNDEF     0

/* Binding type information <--> st_info field in the symbol table */

#define ELF32_ST_BIND(i) ((i)>>4) 
#define ELF32_ST_TYPE(i) ((i)&0xf) 
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

/* Different types of bindings */

#define STB_LOCAL     0
#define STB_GLOBAL    1
#define STB_WEAK      2
#define STB_LOPROC    13
#define STB_HIPROC    15

/* Symbol types */

#define STT_NOTYPE    0
#define STT_OBJECT    1
#define STT_FUNC      2
#define STT_SECTION   3
#define STT_FILE      4
#define STT_LOPROC    13
#define STT_HIPROC    15

/* Relocation Entries */

typedef struct {
	Elf32_Addr    r_offset;
	Elf32_Word    r_info;
} Elf32_Rel;

typedef struct {
	Elf32_Addr    r_offset;
	Elf32_Word    r_info;
	Elf32_Sword   r_addend;
} Elf32_Rela;

/* Symbol table index and relocation type <--> r_info field in the relocation entry  */

#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

/* Relocation types  */

#define R_386_NONE    0
#define R_386_32      1
#define R_386_PC32    2

extern bool      Elf32_IsValidELF              (uintptr_t image);
extern bool      Elf32_IsValidExecutable       (uintptr_t image);
extern bool      Elf32_IsValidRelocatable      (uintptr_t image);
extern bool      Elf32_IsValidiStaticExecutable(uintptr_t image);
extern bool      Elf32_LoadStaticExecutable    (uintptr_t image, size_t size, uintptr_t start_addr);
extern bool      Elf32_LoadBSSLikeSections     (uintptr_t image, uintptr_t start_addr);
extern bool      Elf32_Relocate                (uintptr_t image);
extern bool      Elf32_RelocateSymbol          (uintptr_t image, Elf32_Rel* rel, Elf32_Shdr* rel_table_hdr);
extern uintptr_t Elf32_GetSymbolValue          (uintptr_t image, uintptr_t table, size_t idx);
extern void*     Elf32_LookupSymbol(const char* name);

#endif
