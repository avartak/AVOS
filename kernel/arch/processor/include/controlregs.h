#ifndef KERNEL_CRS_H
#define KERNEL_CRS_H

#include <stdint.h>

#define CR0_PE                  1
#define CR0_MP                  2
#define CR0_EM                  4
#define CR0_TS                  8
#define CR0_ET                  0x10
#define CR0_NE                  0x20
#define CR0_WP                  0x10000
#define CR0_AM                  0x40000
#define CR0_NW                  0x20000000
#define CR0_CD                  0x40000000
#define CR0_PG                  0x80000000

#define CR4_VME                 1
#define CR4_PVI                 2
#define CR4_TSD                 4
#define CR4_DE                  8
#define CR4_PSE                 0x10
#define CR4_PAE                 0x20
#define CR4_MCE                 0x40
#define CR4_PGE                 0x80
#define CR4_PCE                 0x100
#define CR4_OSFXSR              0x200
#define CR4_OSXMMEXCPT          0x400
#define CR4_UMIP                0x800
#define CR4_LA57                0x1000
#define CR4_VMXE                0x2000
#define CR4_SMXE                0x4000
#define CR4_FSGSBASE            0x10000
#define CR4_PCIDE               0x20000
#define CR4_OSXSAVE             0x40000
#define CR4_SMEP                0x100000
#define CR4_SMAP                0x200000
#define CR4_PKE                 0x400000

extern void     CR0_Write(uint32_t value); 
extern void     CR3_Write(uint32_t value); 
extern void     CR4_Write(uint32_t value); 

extern uint32_t CR0_Read();
extern uint32_t CR2_Read();
extern uint32_t CR3_Read();
extern uint32_t CR4_Read();

#endif
