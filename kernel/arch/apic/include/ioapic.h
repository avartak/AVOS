#ifndef KERNEL_IOAPIC_H
#define KERNEL_IOAPIC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define IOAPIC_REG_ID                 0
#define IOAPIC_REG_VERSION            1
#define IOAPIC_INTR_MASKED            0x10000
#define IOAPIC_REDIRECT_TABLE_BASE    0x10

struct IOAPIC_RW {
	uint32_t reg;
	uint32_t padding[3];
	uint32_t data;
};

extern size_t    IOAPIC_Num;
extern uintptr_t IOAPIC_InfoPtrs[];

extern bool      IOAPIC_Initialize();
extern void      IOAPIC_WriteTo(volatile struct IOAPIC_RW* ioapic, size_t reg, uint32_t value);
extern uint32_t  IOAPIC_ReadFrom(volatile struct IOAPIC_RW* ioapic, size_t reg);
extern void      IOAPIC_EnableInterrupt(uint8_t irq, uint8_t intr_vec, uint8_t local_apic_id);

#endif
