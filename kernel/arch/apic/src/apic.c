#include <kernel/arch/apic/include/apic.h>
#include <kernel/arch/apic/include/pic.h>
#include <kernel/arch/acpi/include/acpi.h>
#include <kernel/initial/include/multiboot.h>
#include <kernel/initial/include/setup.h>

extern struct Multiboot_Info_Start* BootInfo_Ptr;

struct ACPI_MADT* APIC_MADT_Ptr;
uintptr_t         APIC_LocalAPIC_Info_Ptrs[MACHINE_MAX_CPUS];
uintptr_t         APIC_IOAPIC_Info_Ptrs[MACHINE_MAX_IOAPICS];

uintptr_t         APIC_LocalAPIC_address;
bool              APIC_LegacyPIC_exists;

size_t            APIC_NumCPUs = 0;
size_t            APIC_IOAPICs = 0;

bool APIC_SaveInfo() {

	struct ACPI_RSDPv1* rsdp = (struct ACPI_RSDPv1*)0;

	struct Multiboot_Info_Tag* mbi_tag;
	uintptr_t mbi_addr = (uintptr_t)BootInfo_Ptr;
	for (mbi_tag = (struct Multiboot_Info_Tag*)(mbi_addr + 8); mbi_tag->type != 0; mbi_tag = (struct Multiboot_Info_Tag*)((uint8_t*)mbi_tag + ((mbi_tag->size + 7) & ~7))) {
		if (mbi_tag->type == MULTIBOOT_TAG_TYPE_ACPI_OLD || mbi_tag->type == MULTIBOOT_TAG_TYPE_ACPI_NEW) {
			struct Multiboot_Info_ACPIv1* mbi_tag_rsdp = (struct Multiboot_Info_ACPIv1*)mbi_tag;
			rsdp = (struct ACPI_RSDPv1*)(&(mbi_tag_rsdp->rsdp));
		}
	}
	if (rsdp == (struct ACPI_RSDPv1*)0) return false;

	uint8_t checksum = 0;

	struct ACPI_RSDT* rsdt = (struct ACPI_RSDT*)(rsdp->rsdt_address + KERNEL_HIGHER_HALF_OFFSET);
	for (size_t i = 0; i < (rsdt->header).length; i++) checksum += ((uint8_t*)(rsdt))[i];
	if (checksum != 0) return false;

	APIC_MADT_Ptr = (struct ACPI_MADT*)0;
	for (size_t i = 0; i < ((rsdt->header).length - sizeof(struct ACPI_SDTHeader))/4; i++) {
		struct ACPI_SDTHeader* sdt = (struct ACPI_SDTHeader*)(rsdt->entry[i] + KERNEL_HIGHER_HALF_OFFSET);
		if (sdt->signature[0] == 'A' && sdt->signature[1] == 'P' && sdt->signature[2] == 'I' && sdt->signature[3] == 'C') APIC_MADT_Ptr = (struct ACPI_MADT*)sdt;
	}
	if (APIC_MADT_Ptr == (struct ACPI_MADT*)0) return false;
	checksum = 0;
	for (size_t j = 0; j < APIC_MADT_Ptr->header.length; j++) checksum += ((uint8_t*)(APIC_MADT_Ptr))[j];
	if (checksum != 0) return false;

	APIC_LocalAPIC_address = APIC_MADT_Ptr->local_apic_address;
	APIC_LegacyPIC_exists  = (bool)APIC_MADT_Ptr->flags;
	if (APIC_LegacyPIC_exists) PIC_Initialize();

	size_t num_cpus    = 0;
	size_t num_ioapics = 0;
	struct ACPI_MADT_Entry* entry = APIC_MADT_Ptr->entries;
	while ((uint32_t)entry < (uint32_t)APIC_MADT_Ptr + APIC_MADT_Ptr->header.length) {
	    if (entry->type == 0) num_cpus++;
	    if (entry->type == 1) num_ioapics++;
	    entry = (struct ACPI_MADT_Entry*)((uint32_t)entry + entry->length);
	}
	if (num_cpus > MACHINE_MAX_CPUS || num_cpus > MACHINE_MAX_IOAPICS) return false;
	APIC_NumCPUs = num_cpus;
	APIC_IOAPICs = num_ioapics;

	num_cpus    = 0;
	num_ioapics = 0;
	entry = APIC_MADT_Ptr->entries;
	while ((uint32_t)entry < (uint32_t)APIC_MADT_Ptr + APIC_MADT_Ptr->header.length) {
        if (entry->type == 0) APIC_LocalAPIC_Info_Ptrs[num_cpus++] = (uintptr_t)entry;
        if (entry->type == 1) APIC_IOAPIC_Info_Ptrs[num_ioapics++] = (uintptr_t)entry;
	    if (entry->type == 5) {
	        struct ACPI_MADT_Entry_LocalAPICAddrOverride* lapic_addr_ovr = (struct ACPI_MADT_Entry_LocalAPICAddrOverride*)entry;
			uint64_t addr = lapic_addr_ovr->local_apic_phys_addr;
			if (addr > ~((uint32_t)0)) return false;
			else APIC_LocalAPIC_address = (uint32_t)addr;
	    }
	    entry = (struct ACPI_MADT_Entry*)((uint32_t)entry + entry->length);
	}

	return true;	
}
 
