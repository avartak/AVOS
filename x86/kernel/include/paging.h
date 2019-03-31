#ifndef X86_KERNEL_PAGING_H
#define X86_KERNEL_PAGING_H

#include <kernel/include/common.h>
#include <kernel/include/paging.h>

uint32_t Paging_directory[0x400]__attribute__((aligned(0x1000)));
uint32_t Paging_kerntable[0x400]__attribute__((aligned(0x1000)));

extern void Paging_Enable();
extern void Paging_Initialize();
extern void Paging_SwitchToHigherHalf();

#endif
