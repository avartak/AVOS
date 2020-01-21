#ifndef KERNEL_ACPI_ACPI_H
#define KERNEL_ACPI_ACPI_H

#include <stdint.h>

struct ACPI_RSDPv1 {
	char     signature[8];
	uint8_t  checksum;
	char     oem_id[6];
	uint8_t  revision;
	uint32_t rsdt_address;
}__attribute__((packed));

struct ACPI_RSDPv2 {
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

struct ACPI_SDTHeader {
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

struct ACPI_RSDT {
	struct ACPI_SDTHeader header;
	uint32_t entry[];
}__attribute__((packed));

struct ACPI_XSDT {
	struct ACPI_SDTHeader header;
	uint64_t entry[];
}__attribute__((packed));

#endif
