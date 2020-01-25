#ifndef KERNEL_MADT_H
#define KERNEL_MADT_H

#include <stdint.h>

#include <kernel/arch/acpi/include/acpi.h>

#define MADT_ENTRY_PROC_LAPIC 0  
#define MADT_ENTRY_IOAPIC     1  
#define MADT_ENTRY_ISO        2  
#define MADT_ENTRY_NMI        4  
#define MADT_ENTRY_LAPIC_ADDR 5

struct MADT_Entry {
	uint8_t  type;
	uint8_t  length;
	uint8_t  contents[];
}__attribute__((packed));

struct MADT_Entry_LocalAPIC {
	uint8_t  type;
	uint8_t  length;
	uint8_t  acpi_proc_id;
	uint8_t  apic_id;
	uint8_t  flags;
}__attribute__((packed));

struct MADT_Entry_IOAPIC {
    uint8_t  type;
    uint8_t  length;
    uint8_t  ioapic_id;
    uint8_t  reserved;
    uint32_t ioapic_address;
    uint32_t global_system_intr_base;
}__attribute__((packed));

struct MADT_Entry_IntrSrcOverride {
    uint8_t  type;
    uint8_t  length;
	uint8_t  bus_source;
	uint8_t  irq_source;
	uint32_t global_sys_intr;
	uint16_t flags;
}__attribute__((packed));

struct MADT_Entry_NMISource {
    uint8_t  type;
    uint8_t  length;
    uint16_t flags;
	uint32_t global_sys_intr;
}__attribute__((packed));

struct MADT_Entry_NMI {
    uint8_t  type;
    uint8_t  length;
    uint8_t  acpi_proc_id;
    uint16_t flags;
    uint8_t  LINT_num;
}__attribute__((packed));

struct MADT_Entry_LocalAPICAddrOverride {
    uint8_t  type;
    uint8_t  length;
    uint16_t reserved;
    uint64_t local_apic_phys_addr;
}__attribute__((packed));

struct MADT {
    struct SDTHeader header;
	uint32_t local_apic_address;
	uint32_t flags;
    struct MADT_Entry entries[];
}__attribute__((packed));

extern struct MADT* MADT_ptr;

#endif
