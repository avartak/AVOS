#ifndef KERNEL_ACPI_H
#define KERNEL_ACPI_H

#include <stdint.h>

struct RSDPv1 {
	char     signature[8];
	uint8_t  checksum;
	char     oem_id[6];
	uint8_t  revision;
	uint32_t rsdt_address;
}__attribute__((packed));

struct RSDPv2 {
    char     signature[8];
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;
    uint32_t rsdt_address;
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t  checksum_extended;
	uint8_t  reserved[3];
}__attribute__((packed));

struct SDTHeader {
	char     signature[4];
	uint32_t length;
	uint8_t  revision;
	uint8_t  checksum;
	char     oem_id[6];
	char     oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;	
}__attribute__((packed));

struct RSDT {
	struct SDTHeader header;
	uint32_t entry[];
}__attribute__((packed));

struct XSDT {
	struct SDTHeader header;
	uint64_t entry[];
}__attribute__((packed));

#endif
