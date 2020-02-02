#include <kernel/arch/apic/include/apic.h>
#include <kernel/arch/apic/include/ioapic.h>
#include <kernel/arch/apic/include/lapic.h>
#include <kernel/arch/apic/include/pic.h>
#include <kernel/arch/acpi/include/madt.h>
#include <kernel/arch/i386/include/functions.h>
#include <kernel/core/setup/include/setup.h>

uintptr_t IOAPIC_InfoPtrs[MACHINE_MAX_IOAPICS];
size_t    IOAPIC_Num = 0;

void IOAPIC_WriteTo(volatile struct IOAPIC_RW* ioapic, size_t index, uint32_t value) {

	ioapic->reg = index;
	ioapic->data = value;

}

uint32_t IOAPIC_ReadFrom(volatile struct IOAPIC_RW* ioapic, size_t index) {

	ioapic->reg = index;
    return ioapic->data;

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
