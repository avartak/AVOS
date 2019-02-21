#ifndef X86_KERNEL_CLOCK_H
#define X86_KERNEL_CLOCK_H

#include <stdint.h>

typedef uint32_t clock_t;

extern clock_t clockticks;

extern void            Clock_HandleInterrupt();
extern uint32_t        Clock_Ticks();

#endif
