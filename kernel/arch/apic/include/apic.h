#ifndef KERNEL_APIC_H
#define KERNEL_APIC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kernel/initial/include/setup.h>
#include <kernel/arch/acpi/include/madt.h>

extern struct ACPI_MADT* APIC_MADT_Ptr;
extern uintptr_t         APIC_LocalAPIC_Info_Ptrs[];
extern uintptr_t         APIC_IOAPIC_Info_Ptrs[];
extern uintptr_t         APIC_LocalAPIC_address;
extern bool              APIC_LegacyPIC_exists;
extern size_t            APIC_NumCPUs;
extern size_t            APIC_IOAPICs;

extern bool              APIC_SaveInfo();
extern bool              APIC_Initialize_LocalAPIC();
extern bool              APIC_Initialize_IOAPICs();

#endif
