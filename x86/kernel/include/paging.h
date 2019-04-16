#ifndef X86_KERNEL_PAGING_H
#define X86_KERNEL_PAGING_H

#include <kernel/include/common.h>
#include <kernel/include/paging.h>

extern void Paging_Enable();
extern void Paging_Initialize();
extern void Paging_SwitchToHigherHalf();

#endif
