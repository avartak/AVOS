#include <kernel/arch/apic/include/apic.h>
#include <kernel/arch/apic/include/pic.h>
#include <kernel/arch/acpi/include/acpi.h>
#include <kernel/arch/acpi/include/madt.h>
#include <kernel/arch/i386/include/functions.h>
#include <kernel/arch/i386/include/ioports.h>
#include <kernel/arch/timer/include/pit.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/multiboot/include/multiboot.h>

bool APIC_InfoSaved = false;

uintptr_t LocalAPIC_address = 0;
size_t    LocalAPIC_Num = 0;
uintptr_t LocalAPIC_InfoPtrs[MACHINE_MAX_CPUS];

uintptr_t IOAPIC_InfoPtrs[MACHINE_MAX_IOAPICS];
size_t    IOAPIC_Num = 0;

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

uint32_t LocalAPIC_WriteTo(size_t index, uint32_t value) {
	
	volatile uint32_t* lapic = (volatile uint32_t*)LocalAPIC_address;
	lapic[index] = value;
	return lapic[LAPIC_REG_ID];
}

uint32_t LocalAPIC_ReadFrom(size_t index) {

    volatile uint32_t* lapic = (volatile uint32_t*)LocalAPIC_address;
    return lapic[index];

}

void IOAPIC_WriteTo(volatile struct IOAPIC_RW* ioapic, size_t index, uint32_t value) {

	ioapic->reg = index;
	ioapic->data = value;

}

uint32_t IOAPIC_ReadFrom(volatile struct IOAPIC_RW* ioapic, size_t index) {

	ioapic->reg = index;
    return ioapic->data;

}

void LocalAPIC_EOI() {

	LocalAPIC_WriteTo(LAPIC_REG_EOI, 0);	 

}

uint8_t LocalAPIC_ID() {
	return LocalAPIC_ReadFrom(LAPIC_REG_ID) >> 24;
}

bool LocalAPIC_Initialize() {

	APIC_SaveInfo();

	// Enable the local APIC by writing to the spurious interrupt vector register
	LocalAPIC_WriteTo(LAPIC_REG_SIVR, LAPIC_SIVR_SOFT_ENABLE | LAPIC_SIVR_INTR_SPURIOUS);

	// Disable the timer, LINT0, LINT1, ERROR local interrupts
	LocalAPIC_WriteTo(LAPIC_REG_TIMER, LAPIC_LVT_MASKED);	 
	LocalAPIC_WriteTo(LAPIC_REG_LINT0, LAPIC_LVT_MASKED);	 
	LocalAPIC_WriteTo(LAPIC_REG_LINT1, LAPIC_LVT_MASKED);	 
	LocalAPIC_WriteTo(LAPIC_REG_ERROR, LAPIC_LVT_MASKED);	 

	// Disable local interrupts from the thermal sensor and performance monitors (if they exist)
	if (((LocalAPIC_ReadFrom(LAPIC_REG_VERSION) >> 16) & 0xFF) > 3) LocalAPIC_WriteTo(LAPIC_REG_PCMR, LAPIC_LVT_MASKED);
	if (((LocalAPIC_ReadFrom(LAPIC_REG_VERSION) >> 16) & 0xFF) > 4) LocalAPIC_WriteTo(LAPIC_REG_TSR , LAPIC_LVT_MASKED);

	// Clear registers used for configuring the local timer
	LocalAPIC_WriteTo(LAPIC_REG_TICR, 0);	 
	LocalAPIC_WriteTo(LAPIC_REG_TDCR, 0);	 

	// Clear the error state register - needs writing twice, once for arming and then for the actual write
	LocalAPIC_WriteTo(LAPIC_REG_ESR, 0);	 
	LocalAPIC_WriteTo(LAPIC_REG_ESR, 0);	 

	// Clear pending interrupts
	LocalAPIC_WriteTo(LAPIC_REG_EOI, 0);	 

	// Broadcast an INIT-LEVEL-DEASSERT to reset the arbitration priority register
	LocalAPIC_WriteTo(LAPIC_REG_ICRHI, 0);	 
	LocalAPIC_WriteTo(LAPIC_REG_ICRLO, LAPIC_ICR_DEST_ALLSELF | LAPIC_LVT_DELIVERY_INIT | LAPIC_ICR_TRIGGER_LEVEL | LAPIC_ICR_LEVEL_DEASSERT);

	// Make sure the broadcast went through
	while (LocalAPIC_ReadFrom(LAPIC_REG_ICRLO) & LAPIC_ICR_DELIVERY_PENDING);

	// Enable all interrupts in the local APIC (this does not apply to the CPU) by setting the task priority register to the lowest possible value i.e. 0
	LocalAPIC_WriteTo(LAPIC_REG_TPR, 0); 

	return true;
}

bool IOAPIC_Initialize() {

	APIC_SaveInfo();

	// If legacy PIC (master/slave) exist then initialize them and disable them
	if (PIC_exists) PIC_Initialize();

	// Mask all IRQs coming to all IOAPICs
	for (size_t i = 0; i < IOAPIC_Num; i++) {
		struct MADT_Entry_IOAPIC* madt_ioapic = (struct MADT_Entry_IOAPIC*)IOAPIC_InfoPtrs[i];
		uint8_t madt_ioapic_id = madt_ioapic->ioapic_id;
		struct IOAPIC_RW* ioapic = (struct IOAPIC_RW*)(madt_ioapic->ioapic_address);
		if (madt_ioapic_id != (IOAPIC_ReadFrom(ioapic, IOAPIC_REG_ID) >> 24)) return false;

		size_t max_interrupts = (IOAPIC_ReadFrom(ioapic, IOAPIC_REG_VERSION) >> 16) & 0xFF;
		for (size_t j = 0; j <= max_interrupts; j++) {
			IOAPIC_WriteTo(ioapic, IOAPIC_REDIRECT_TABLE_BASE+2*j  , IOAPIC_INTR_MASKED);
			IOAPIC_WriteTo(ioapic, IOAPIC_REDIRECT_TABLE_BASE+2*j+1, LocalAPIC_ReadFrom(LAPIC_REG_ID) >> 24);
		}
	}	

	// Enable interrupts on this CPU
	X86_EnableInterrupts();

    return true;
}

void IOAPIC_EnableInterrupt(uint8_t irq, uint8_t intr_vec, uint8_t local_apic_id) {

	// Find the global system interrupt corresponding to an IRQ and its signal type (polarity, edge/level triggered)
	uint32_t global_sys_intr = (uint32_t)irq;
	uint16_t flags = 0;

	struct MADT_Entry* entry = MADT_ptr->entries;
    while ((uint32_t)entry < (uint32_t)MADT_ptr + MADT_ptr->header.length) {
        if (entry->type == 2) {
	        struct MADT_Entry_IntrSrcOverride* intr_src_ovr = (struct MADT_Entry_IntrSrcOverride*)entry;
			if (intr_src_ovr->irq_source == irq) {
				global_sys_intr = intr_src_ovr->global_sys_intr;
				flags = intr_src_ovr->flags;
			}
		}
        entry = (struct MADT_Entry*)((uint32_t)entry + entry->length);
    }

	// Direct the IRQ to a given local APIC (CPU)
    for (size_t i = 0; i < IOAPIC_Num; i++) {
        struct MADT_Entry_IOAPIC* madt_ioapic = (struct MADT_Entry_IOAPIC*)IOAPIC_InfoPtrs[i];
        volatile struct IOAPIC_RW* ioapic = (volatile struct IOAPIC_RW*)(madt_ioapic->ioapic_address);
        size_t max_interrupts = (IOAPIC_ReadFrom(ioapic, IOAPIC_REG_VERSION) >> 16) & 0xFF;

		if (madt_ioapic->global_system_intr_base <= global_sys_intr && madt_ioapic->global_system_intr_base + max_interrupts >= global_sys_intr) {
			uint32_t intr_idx = global_sys_intr - madt_ioapic->global_system_intr_base;
        	IOAPIC_WriteTo(ioapic, IOAPIC_REDIRECT_TABLE_BASE+2*intr_idx  , MADT_POLARITY(flags) | MADT_POLARITY(flags) | intr_vec);
        	IOAPIC_WriteTo(ioapic, IOAPIC_REDIRECT_TABLE_BASE+2*intr_idx+1, local_apic_id);
			break;
		}
    }

}
