#ifndef KERNEL_SMP_H
#define KERNEL_SMP_H

#include <stdint.h>
#include <stdbool.h>

extern bool SMP_Initialize_CPU(uint8_t local_apic_id, uint32_t boot_address);

#endif
