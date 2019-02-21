#include <kernel/include/clock.h>
#include <x86/kernel/include/welcome.h>

clock_t clockticks = 0;

void Clock_HandleInterrupt() {
	clockticks++;
	PrintNum(clockticks/50, 24, 0);
}

uint32_t Clock_Ticks() {
	return clockticks;
}
