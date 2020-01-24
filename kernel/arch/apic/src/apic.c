#include <kernel/arch/apic/include/apic.h>
#include <kernel/arch/apic/include/pic.h>
#include <kernel/arch/acpi/include/acpi.h>
#include <kernel/arch/i386/include/functions.h>
#include <kernel/arch/i386/include/ioports.h>
#include <kernel/arch/timer/include/pit.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/multiboot/include/multiboot.h>

extern struct Multiboot_Info_Start* BootInfo_Ptr;

struct ACPI_MADT* APIC_MADT_Ptr;

bool      APIC_LegacyPIC_exists;

uintptr_t APIC_Local_address = 0;
size_t    APIC_Local_Num = 0;
uintptr_t APIC_Local_InfoPtrs[MACHINE_MAX_CPUS];

uintptr_t APIC_IO_InfoPtrs[MACHINE_MAX_IOAPICS];
size_t    APIC_IO_Num = 0;

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

	APIC_Local_address = APIC_MADT_Ptr->local_apic_address;
	APIC_LegacyPIC_exists  = (bool)APIC_MADT_Ptr->flags;

	size_t num_cpus    = 0;
	size_t num_ioapics = 0;
	struct ACPI_MADT_Entry* entry = APIC_MADT_Ptr->entries;
	while ((uint32_t)entry < (uint32_t)APIC_MADT_Ptr + APIC_MADT_Ptr->header.length) {
	    if (entry->type == 0) num_cpus++;
	    if (entry->type == 1) num_ioapics++;
	    entry = (struct ACPI_MADT_Entry*)((uint32_t)entry + entry->length);
	}
	if (num_cpus > MACHINE_MAX_CPUS || num_cpus > MACHINE_MAX_IOAPICS) return false;
	APIC_Local_Num = num_cpus;
	APIC_IO_Num = num_ioapics;

	num_cpus    = 0;
	num_ioapics = 0;
	entry = APIC_MADT_Ptr->entries;
	while ((uint32_t)entry < (uint32_t)APIC_MADT_Ptr + APIC_MADT_Ptr->header.length) {
        if (entry->type == 0) APIC_Local_InfoPtrs[num_cpus++] = (uintptr_t)entry;
        if (entry->type == 1) APIC_IO_InfoPtrs[num_ioapics++] = (uintptr_t)entry;
	    if (entry->type == 5) {
	        struct ACPI_MADT_Entry_LocalAPICAddrOverride* lapic_addr_ovr = (struct ACPI_MADT_Entry_LocalAPICAddrOverride*)entry;
			uint64_t addr = lapic_addr_ovr->local_apic_phys_addr;
			if (addr > ~((uint32_t)0)) return false;
			else APIC_Local_address = (uint32_t)addr;
	    }
	    entry = (struct ACPI_MADT_Entry*)((uint32_t)entry + entry->length);
	}

	return true;	
}

uint32_t APIC_Local_WriteTo(size_t index, uint32_t value) {
	
	volatile uint32_t* lapic = (volatile uint32_t*)APIC_Local_address;
	lapic[index] = value;
	return lapic[0x20/4];
}

uint32_t APIC_Local_ReadFrom(size_t index) {

    volatile uint32_t* lapic = (volatile uint32_t*)APIC_Local_address;
    return lapic[index];

}

void APIC_IO_WriteTo(volatile struct APIC_IO_RW* ioapic, size_t index, uint32_t value) {

	ioapic->reg = index;
	ioapic->data = value;

}

uint32_t APIC_IO_ReadFrom(volatile struct APIC_IO_RW* ioapic, size_t index) {

	ioapic->reg = index;
    return ioapic->data;

}

void APIC_Local_EOI() {

	APIC_Local_WriteTo(0x0B0/4, 0);	 

}

uint8_t APIC_Local_ID() {
	return APIC_Local_ReadFrom(0x20/4) >> 24;
}

bool APIC_Local_Initialize() {

	APIC_Local_WriteTo(0xF0/4, 0x1FF);

	APIC_Local_WriteTo(0x320/4, 0x10000);	 
	APIC_Local_WriteTo(0x350/4, 0x10000);	 
	APIC_Local_WriteTo(0x360/4, 0x10000);	 
	APIC_Local_WriteTo(0x370/4, 0x10000);	 

	if (APIC_Local_ReadFrom(0x30/4) > 3) APIC_Local_WriteTo(0x340/4, 0x10000);
	if (APIC_Local_ReadFrom(0x30/4) > 4) APIC_Local_WriteTo(0x330/4, 0x10000);

	APIC_Local_WriteTo(0x380/4, 0);	 
	APIC_Local_WriteTo(0x3E0/4, 0);	 

	APIC_Local_WriteTo(0x280/4, 0);	 
	APIC_Local_WriteTo(0x280/4, 0);	 

	APIC_Local_WriteTo(0x0B0/4, 0);	 

	APIC_Local_WriteTo(0x310/4, 0);	 
	APIC_Local_WriteTo(0x300/4, 0x80000 | 0x500 | 0x8000);
	while (APIC_Local_ReadFrom(0x300/4) & 0x1000);

	APIC_Local_WriteTo(0x080/4, 0); 

	return true;
}

bool APIC_IO_Initializes() {

	if (APIC_LegacyPIC_exists) PIC_Initialize();

	for (size_t i = 0; i < APIC_IO_Num; i++) {
		struct ACPI_MADT_Entry_IOAPIC* madt_ioapic = (struct ACPI_MADT_Entry_IOAPIC*)APIC_IO_InfoPtrs[i];
		uint8_t madt_ioapic_id = madt_ioapic->ioapic_id;
		struct APIC_IO_RW* ioapic = (struct APIC_IO_RW*)(madt_ioapic->ioapic_address);
		if (madt_ioapic_id != (APIC_IO_ReadFrom(ioapic, 0) >> 24)) return false;

		size_t max_interrupts = (APIC_IO_ReadFrom(ioapic, 0)) >> 24;
		for (size_t j = 0; j <= max_interrupts; j++) {
			APIC_IO_WriteTo(ioapic, 0x10+2*j  , 0x10000);
			APIC_IO_WriteTo(ioapic, 0x10+2*j+1, 0);
		}
	}	

	X86_EnableInterrupts();

    return true;
}

void APIC_IO_EnableInterrupt(uint8_t irq, uint8_t intr_vec, uint8_t local_apic_id) {

	uint32_t global_sys_intr = (uint32_t)irq;
	uint16_t flags = 0;

	struct ACPI_MADT_Entry* entry = APIC_MADT_Ptr->entries;
    while ((uint32_t)entry < (uint32_t)APIC_MADT_Ptr + APIC_MADT_Ptr->header.length) {
        if (entry->type == 2) {
	        struct ACPI_MADT_Entry_IntrSrcOverride* intr_src_ovr = (struct ACPI_MADT_Entry_IntrSrcOverride*)entry;
			if (intr_src_ovr->irq_source == irq) {
				global_sys_intr = intr_src_ovr->global_sys_intr;
				flags = intr_src_ovr->flags;
			}
		}
        entry = (struct ACPI_MADT_Entry*)((uint32_t)entry + entry->length);
    }

	uint32_t polarity = ((flags & 2) == 0 ? 0 : 1 << 13);
	uint32_t trigmode = ((flags & 8) == 0 ? 0 : 1 << 15);
    for (size_t i = 0; i < APIC_IO_Num; i++) {
        struct ACPI_MADT_Entry_IOAPIC* madt_ioapic = (struct ACPI_MADT_Entry_IOAPIC*)APIC_IO_InfoPtrs[i];
        volatile struct APIC_IO_RW* ioapic = (volatile struct APIC_IO_RW*)(madt_ioapic->ioapic_address);
        size_t max_interrupts = (APIC_IO_ReadFrom(ioapic, 1) >> 16) & 0xFF;

		if (madt_ioapic->global_system_intr_base <= global_sys_intr && madt_ioapic->global_system_intr_base + max_interrupts >= global_sys_intr) {
			uint32_t intr_idx = global_sys_intr - madt_ioapic->global_system_intr_base;
        	APIC_IO_WriteTo(ioapic, 0x10+2*intr_idx  , polarity | trigmode | intr_vec);
        	APIC_IO_WriteTo(ioapic, 0x10+2*intr_idx+1, local_apic_id << 24);
			break;
		}
    }

}
