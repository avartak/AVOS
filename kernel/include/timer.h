#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include <kernel/include/common.h>

extern clock_t timerticks;

extern uint32_t Timer_Ticks();
extern uint32_t Timer_HandleInterrupt();
extern void     Timer_Initialize();

#endif
