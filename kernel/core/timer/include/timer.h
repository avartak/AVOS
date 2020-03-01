#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

#include <kernel/core/setup/include/setup.h>

struct Timer {
	atomic_size_t  seq_counter;
	atomic_clock_t ticks;
};

void    Timer_Increment(struct Timer* tmr);
clock_t Timer_GetTicks(struct Timer* tmr); 

#endif
