#ifndef KERNEL_X86_CRS_H
#define KERNEL_X86_CRS_H

#include <stdint.h>

#define X86_CR0_PE                  1
#define X86_CR0_MP                  2
#define X86_CR0_EM                  4
#define X86_CR0_TS                  8
#define X86_CR0_ET                  0x10
#define X86_CR0_NE                  0x20
#define X86_CR0_WP                  0x10000
#define X86_CR0_AM                  0x40000
#define X86_CR0_NW                  0x20000000
#define X86_CR0_CD                  0x40000000
#define X86_CR0_PG                  0x80000000

#define X86_CR4_VME                 1
#define X86_CR4_PVI                 2
#define X86_CR4_TSD                 4
#define X86_CR4_DE                  8
#define X86_CR4_PSE                 0x10
#define X86_CR4_PAE                 0x20
#define X86_CR4_MCE                 0x40
#define X86_CR4_PGE                 0x80
#define X86_CR4_PCE                 0x100
#define X86_CR4_OSFXSR              0x200
#define X86_CR4_OSXMMEXCPT          0x400
#define X86_CR4_UMIP                0x800
#define X86_CR4_LA57                0x1000
#define X86_CR4_VMXE                0x2000
#define X86_CR4_SMXE                0x4000
#define X86_CR4_FSGSBASE            0x10000
#define X86_CR4_PCIDE               0x20000
#define X86_CR4_OSXSAVE             0x40000
#define X86_CR4_SMEP                0x100000
#define X86_CR4_SMAP                0x200000
#define X86_CR4_PKE                 0x400000

extern void     X86_CR0_Write(uint32_t value); 
extern void     X86_CR3_Write(uint32_t value); 
extern void     X86_CR4_Write(uint32_t value); 

extern uint32_t X86_CR0_Read();
extern uint32_t X86_CR2_Read();
extern uint32_t X86_CR3_Read();
extern uint32_t X86_CR4_Read();

#endif
