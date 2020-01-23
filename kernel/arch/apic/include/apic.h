#ifndef KERNEL_APIC_H
#define KERNEL_APIC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kernel/core/setup/include/setup.h>
#include <kernel/arch/acpi/include/madt.h>

struct APIC_IO_RW {
	uint32_t reg;
	uint32_t padding[3];
	uint32_t data;
};

extern struct ACPI_MADT* APIC_MADT_Ptr;

extern bool      APIC_LegacyPIC_exists;
extern bool      APIC_SaveInfo();

extern uintptr_t APIC_Local_address;
extern size_t    APIC_Local_Num;
extern uintptr_t APIC_Local_InfoPtrs[];

extern bool      APIC_Local_Initialize();
extern uint32_t  APIC_Local_WriteTo(size_t index, uint32_t value);
extern uint32_t  APIC_Local_ReadFrom(size_t index);
extern void      APIC_Local_EOI();
extern uint8_t   APIC_Local_ID();

extern size_t    APIC_IO_Num;
extern uintptr_t APIC_IO_InfoPtrs[];

extern bool      APIC_IO_Initializes();
extern void      APIC_IO_WriteTo(volatile struct APIC_IO_RW* ioapic, size_t reg, uint32_t value);
extern uint32_t  APIC_IO_ReadFrom(volatile struct APIC_IO_RW* ioapic, size_t reg);
extern void      APIC_IO_EnableInterrupt(uint8_t irq, uint8_t intr_vec, uint8_t local_apic_id);

#endif
