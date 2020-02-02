#include <kernel/arch/apic/include/apic.h>
#include <kernel/arch/apic/include/ioapic.h>
#include <kernel/arch/apic/include/lapic.h>
#include <kernel/arch/apic/include/pic.h>
#include <kernel/arch/acpi/include/acpi.h>
#include <kernel/arch/acpi/include/madt.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/multiboot/include/multiboot.h>

bool APIC_InfoSaved = false;

bool APIC_SaveInfo() {

	if (APIC_InfoSaved) return true;

	struct RSDPv1* rsdp = (struct RSDPv1*)0;

	extern struct Multiboot_Info_Start* BootInfo_Ptr;

	struct Multiboot_Info_Tag* mbi_tag;
	uintptr_t mbi_addr = (uintptr_t)BootInfo_Ptr;
	for (mbi_tag = (struct Multiboot_Info_Tag*)(mbi_addr + 8); mbi_tag->type != 0; mbi_tag = (struct Multiboot_Info_Tag*)((uint8_t*)mbi_tag + ((mbi_tag->size + 7) & ~7))) {
		if (mbi_tag->type == MULTIBOOT_TAG_TYPE_ACPI_OLD || mbi_tag->type == MULTIBOOT_TAG_TYPE_ACPI_NEW) {
			struct Multiboot_Info_ACPIv1* mbi_tag_rsdp = (struct Multiboot_Info_ACPIv1*)mbi_tag;
			rsdp = (struct RSDPv1*)(&(mbi_tag_rsdp->rsdp));
		}
	}
	if (rsdp == (struct RSDPv1*)0) return false;

	uint8_t checksum = 0;

	struct RSDT* rsdt = (struct RSDT*)(rsdp->rsdt_address + KERNEL_HIGHER_HALF_OFFSET);
	for (size_t i = 0; i < (rsdt->header).length; i++) checksum += ((uint8_t*)(rsdt))[i];
	if (checksum != 0) return false;

	MADT_ptr = (struct MADT*)0;
	for (size_t i = 0; i < ((rsdt->header).length - sizeof(struct SDTHeader))/4; i++) {
		struct SDTHeader* sdt = (struct SDTHeader*)(rsdt->entry[i] + KERNEL_HIGHER_HALF_OFFSET);
		if (sdt->signature[0] == 'A' && sdt->signature[1] == 'P' && sdt->signature[2] == 'I' && sdt->signature[3] == 'C') MADT_ptr = (struct MADT*)sdt;
	}
	if (MADT_ptr == (struct MADT*)0) return false;
	checksum = 0;
	for (size_t j = 0; j < MADT_ptr->header.length; j++) checksum += ((uint8_t*)(MADT_ptr))[j];
	if (checksum != 0) return false;

	PIC_exists = (bool)MADT_ptr->flags;
	LocalAPIC_address = MADT_ptr->local_apic_address;

	size_t num_cpus    = 0;
	size_t num_ioapics = 0;
	struct MADT_Entry* entry = MADT_ptr->entries;
	while ((uint32_t)entry < (uint32_t)MADT_ptr + MADT_ptr->header.length) {
	    if (entry->type == MADT_ENTRY_PROC_LAPIC) num_cpus++;
	    if (entry->type == MADT_ENTRY_IOAPIC) num_ioapics++;
	    entry = (struct MADT_Entry*)((uint32_t)entry + entry->length);
	}
	if (num_cpus > MACHINE_MAX_CPUS || num_cpus > MACHINE_MAX_IOAPICS) return false;
	LocalAPIC_Num = num_cpus;
	IOAPIC_Num = num_ioapics;

	num_cpus    = 0;
	num_ioapics = 0;
	entry = MADT_ptr->entries;
	while ((uint32_t)entry < (uint32_t)MADT_ptr + MADT_ptr->header.length) {
        if (entry->type == MADT_ENTRY_PROC_LAPIC) LocalAPIC_InfoPtrs[num_cpus++] = (uintptr_t)entry;
        if (entry->type == MADT_ENTRY_IOAPIC) IOAPIC_InfoPtrs[num_ioapics++] = (uintptr_t)entry;
	    if (entry->type == MADT_ENTRY_LAPIC_ADDR) {
	        struct MADT_Entry_LocalAPICAddrOverride* lapic_addr_ovr = (struct MADT_Entry_LocalAPICAddrOverride*)entry;
			uint64_t addr = lapic_addr_ovr->local_apic_phys_addr;
			if (addr > ~((uint32_t)0)) return false;
			else LocalAPIC_address = (uint32_t)addr;
	    }
	    entry = (struct MADT_Entry*)((uint32_t)entry + entry->length);
	}

	APIC_InfoSaved = true;
	return true;	
}

