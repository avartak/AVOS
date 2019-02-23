#ifndef KERNEL_CLOCK_H
#define KERNEL_CLOCK_H

#include <stdint.h>

typedef uint32_t clock_t;

extern clock_t clockticks;

extern uint32_t Clock_HandleInterrupt();
extern uint32_t Clock_Ticks();

#endif
