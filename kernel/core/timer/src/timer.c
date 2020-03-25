#include <kernel/core/timer/include/timer.h>

void Timer_Increment(struct Timer* tmr) {

	atomic_fetch_add_explicit(&(tmr->seq_counter), 1, memory_order_relaxed);
	atomic_signal_fence(memory_order_release);

	atomic_fetch_add_explicit(&(tmr->ticks), 1, memory_order_relaxed);
	atomic_signal_fence(memory_order_release);

	atomic_fetch_add_explicit(&(tmr->seq_counter), 1, memory_order_relaxed);
	atomic_signal_fence(memory_order_release);
}

clock_t Timer_GetTicks(struct Timer* tmr) {

	clock_t ticks    = 0;
    size_t  tickseq0 = 0;
    size_t  tickseq1 = 0;

    do {
        tickseq0 = atomic_load_explicit(&(tmr->seq_counter), memory_order_relaxed);
        atomic_signal_fence(memory_order_acquire);

        ticks = atomic_load_explicit(&(tmr->ticks), memory_order_relaxed);
        atomic_signal_fence(memory_order_acquire);

        tickseq1 = atomic_load_explicit(&(tmr->seq_counter), memory_order_relaxed);
        atomic_signal_fence(memory_order_acquire);
    }
    while (tickseq0 != tickseq1 || (tickseq0 & 1));

	return ticks;
}
