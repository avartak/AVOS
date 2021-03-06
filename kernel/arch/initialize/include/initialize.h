#ifndef KERNEL_INITIALIZE_H
#define KERNEL_INITIALIZE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SMP_CMOS_RAM_ADDR_PORT     0x70     
#define SMP_CMOS_RAM_DATA_PORT     0x71

#define SMP_WARM_RESET_ADDR        0x0F
#define SMP_WARM_RESET_SIGN        0x0A
#define SMP_WARM_RESET_VECTOR      (0x40<<4 | 0x67)

#define SMP_WARM_RESET_BOOT_OFF(x) (x  & 0xF)  
#define SMP_WARM_RESET_BOOT_SEG(x) (x >> 4)  

#define SMP_STARTUP_TRIES          2

struct Process;

extern size_t   Kernel_numcpus_online; 
extern uint32_t Kernel_pagedirectory[];
extern size_t   Kernel_stack_offset;

extern void   Initialize_Memory();
extern bool   Initialize_CPU(uint8_t local_apic_id, uint32_t boot_address);
extern void   Initialize_ThisProcessor();
extern void   Initialize_System();

extern void GetToWork()__attribute__ ((noreturn));

#endif
