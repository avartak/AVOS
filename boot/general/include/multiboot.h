#ifndef BOOT_GENERAL_MULTIBOOT_H
#define BOOT_GENERAL_MULTIBOOT_H

#include <boot/general/include/common.h>
#include <boot/general/include/ELF.h>

#define MULTIBOOT_TAG_ALIGN                        8
#define MULTIBOOT_TAG_TYPE_END                     0
#define MULTIBOOT_TAG_TYPE_CMDLINE                 1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME        2
#define MULTIBOOT_TAG_TYPE_MODULE                  3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO           4
#define MULTIBOOT_TAG_TYPE_BOOTDEV                 5
#define MULTIBOOT_TAG_TYPE_MMAP                    6
#define MULTIBOOT_TAG_TYPE_VBE                     7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER             8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS            9
#define MULTIBOOT_TAG_TYPE_APM                     10
#define MULTIBOOT_TAG_TYPE_EFI32                   11
#define MULTIBOOT_TAG_TYPE_EFI64                   12
#define MULTIBOOT_TAG_TYPE_SMBIOS                  13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD                14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW                15
#define MULTIBOOT_TAG_TYPE_NETWORK                 16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP                17
#define MULTIBOOT_TAG_TYPE_EFI_BS                  18
#define MULTIBOOT_TAG_TYPE_EFI32_IH                19
#define MULTIBOOT_TAG_TYPE_EFI64_IH                20
#define MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR          21
#define MULTIBOOT_TAG_TYPE_RAM_INFO                98
#define MULTIBOOT_TAG_TYPE_RAM_INFO_PAGE_ALIGNED   99

#define MULTIBOOT_HEADER_TAG_END                   0
#define MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST   1
#define MULTIBOOT_HEADER_TAG_ADDRESS               2
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS         3
#define MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS         4
#define MULTIBOOT_HEADER_TAG_FRAMEBUFFER           5
#define MULTIBOOT_HEADER_TAG_MODULE_ALIGN          6
#define MULTIBOOT_HEADER_TAG_EFI_BS                7
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32   8
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64   9
#define MULTIBOOT_HEADER_TAG_RELOCATABLE           10

#define MULTIBOOT_ARCHITECTURE_I386                0
#define MULTIBOOT_ARCHITECTURE_MIPS32              4
#define MULTIBOOT_HEADER_TAG_OPTIONAL              1

#define MULTIBOOT_LOAD_PREFERENCE_NONE             0
#define MULTIBOOT_LOAD_PREFERENCE_LOW              1
#define MULTIBOOT_LOAD_PREFERENCE_HIGH             2

#define MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED   1
#define MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED 2

#define MULTIBOOT_MEMORY_ACPI3_FLAG                1
#define MULTIBOOT_MEMORY_AVAILABLE                 1
#define MULTIBOOT_MEMORY_RESERVED                  2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE          3
#define MULTIBOOT_MEMORY_NVS                       4
#define MULTIBOOT_MEMORY_BADRAM                    5

#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED         0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB             1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT        2

struct Multiboot_E820_Entry {
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t acpi3;
}__attribute__((packed));

struct Multiboot_RSDPv1 {
    char     signature[8];
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;
    uint32_t rsdt_address;
}__attribute__((packed));

struct Multiboot_RSDPv2 {
    char     signature[8];
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t  extended_checksum;
    uint8_t  reserved[3];
}__attribute__ ((packed));

struct Multiboot_SMBIOS_EntryPointTable {
    char     entry_point_string[4];
    uint8_t  checksum;
    uint8_t  length;
    uint8_t  major_version;
    uint8_t  minor_version;
    uint16_t max_structure_size;
    uint8_t  entry_point_revision;
    char     formatted_area[5];
    char     entry_point_string2[2];
    uint8_t  checksum2;
    uint16_t table_length;
    uint32_t table_address;
    uint16_t number_of_structures;
    uint8_t  revision_BCD;
}__attribute__((packed));

struct Multiboot_APM_Interface {
	uint16_t version;
	uint16_t cseg;
	uint32_t offset;
	uint16_t cseg_16;
	uint16_t dseg;
	uint16_t flags;
	uint16_t cseg_len;
	uint16_t cseg_16_len;
	uint16_t dseg_len;
}__attribute__((packed));

struct Multiboot_Header_Magic_Fields {
    uint32_t magic;
    uint32_t architecture;
    uint32_t header_length;
    uint32_t checksum;
}__attribute__((packed));

struct Multiboot_Header_Tag {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
}__attribute__((packed));

struct Multiboot_Header_Tag_Information {
	uint16_t type;
	uint16_t flags;
	uint32_t size;
	uint32_t requests[];
}__attribute__((packed));

struct Multiboot_Header_Tag_Address {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t header_addr;
    uint32_t load_addr;
    uint32_t load_end_addr;
    uint32_t bss_end_addr;
}__attribute__((packed));

struct Multiboot_Header_Tag_Entry_Address {
	uint16_t type;
	uint16_t flags;
	uint32_t size;
	uint32_t entry_addr;
}__attribute__((packed));

struct Multiboot_Header_Tag_Console {
	uint16_t type;
	uint16_t flags;
	uint32_t size;
	uint32_t console_flags;
}__attribute__((packed));

struct Multiboot_Header_Tag_Framebuffer {
	uint16_t type;
	uint16_t flags;
	uint32_t size;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
}__attribute__((packed));

struct Multiboot_Header_Tag_Relocatable {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t min_addr;
    uint32_t max_addr;
    uint32_t align;
    uint32_t preference;
}__attribute__((packed));

struct Multiboot_Info_Start {
    uint32_t total_size;
    uint32_t reserved;
}__attribute__((packed));

struct Multiboot_Info_Tag {
	uint32_t type;
	uint32_t size;
}__attribute__((packed));

struct Multiboot_Info_Name {
	uint32_t type;
	uint32_t size;
	char string[];
}__attribute__((packed));

struct Multiboot_Info_Command {
    uint32_t type;
    uint32_t size;
    char string[];
}__attribute__((packed));

struct Multiboot_Info_Modules {
	uint32_t type;
	uint32_t size;
	uint32_t mod_start;
	uint32_t mod_end;
	char string[];
}__attribute__((packed));

struct Multiboot_Info_Memory_Basic {
	uint32_t type;
	uint32_t size;
	uint32_t mem_lower;
	uint32_t mem_upper;
}__attribute__((packed));

struct Multiboot_Info_Memory_E820 {
	uint32_t type;
	uint32_t size;
	uint32_t entry_size;
	uint32_t entry_version;
	struct Multiboot_E820_Entry entries[];
}__attribute__((packed));

struct Multiboot_Info_VBE {
	uint32_t type;
	uint32_t size;
	
	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;

	uint8_t vbe_control_info[512];
	uint8_t vbe_mode_info[256];
}__attribute__((packed));

struct Multiboot_Info_Framebuffer_Common {
	uint32_t type;
	uint32_t size;
	
	uint64_t framebuffer_addr;
	uint32_t framebuffer_pitch;
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
	uint8_t  framebuffer_bpp;
	uint8_t  framebuffer_type;
	uint16_t reserved;
}__attribute__((packed));

struct Multiboot_Info_Color {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
}__attribute__((packed));

struct Multiboot_Info_Framebuffer {
    uint32_t type;
    uint32_t size;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    uint16_t reserved;

	union {
		struct {
			uint16_t framebuffer_palette_num_colors;
			struct Multiboot_Info_Color framebuffer_palette[];
		};
		struct {
			uint8_t framebuffer_red_field_position;
			uint8_t framebuffer_red_mask_size;
			uint8_t framebuffer_green_field_position;
			uint8_t framebuffer_green_mask_size;
			uint8_t framebuffer_blue_field_position;
			uint8_t framebuffer_blue_mask_size;
			uint16_t framebuffer_padding;
		};
	};
}__attribute__((packed));

struct Multiboot_Info_ELF_Sections {
	uint32_t type;
	uint32_t size;
	uint16_t num;
	uint16_t entsize;
	uint16_t shndx;
	uint16_t reserved;
	Elf32_Shdr sections[];
}__attribute__((packed));

struct Multiboot_Info_APM {
	uint32_t type;
	uint32_t size;
	struct Multiboot_APM_Interface apm_interface;
	uint16_t reserved1;
	uint16_t reserved2;
}__attribute__((packed));

struct Multiboot_Info_SMBIOS {
	uint32_t type;
	uint32_t size;
	uint8_t  major;	
	uint8_t  minor;
	uint8_t  reserved[6];
	uint8_t  tables[];
}__attribute__((packed));

struct Multiboot_Info_ACPIv1 {
	uint32_t type;
	uint32_t size;
	struct Multiboot_RSDPv1 rsdp;
	uint32_t reserved;
}__attribute__((packed));

struct Multiboot_Info_ACPIv2 {
	uint32_t type;
	uint32_t size;
	struct Multiboot_RSDPv2 rsdp;
	uint32_t reserved;
}__attribute__((packed));

struct Multiboot_Info_LoadBaseAddress {
	uint32_t type;
	uint32_t size;
	uint32_t load_base_addr;
	uint32_t reserved;
}__attribute__((packed));

struct Multiboot_Info_BootDevice {
    uint32_t type;
    uint32_t size;
    uint32_t biosdev;
    uint32_t partition;
    uint32_t sub_partition;
    uint32_t reserved;
}__attribute__((packed));

#endif
