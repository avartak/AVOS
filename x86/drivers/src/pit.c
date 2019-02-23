#include <x86/drivers/include/pit.h>
#include <x86/drivers/include/pic.h>
#include <x86/drivers/include/io.h>
#include <kernel/include/interrupts.h>
#include <kernel/include/timer.h>

void PIT_Initialize(uint32_t tick_freq) {
	uint32_t pit_counter = PIT_BASE_FREQUENCY/tick_freq;

	Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_SQWAVE_GENERATOR | PIT_ACCESS_LOHIBYTE | PIT_CHANNEL_0);
	Outb(PIT_IOPORT_CHAN0, (uint8_t) (pit_counter & 0x00FF));
	Outb(PIT_IOPORT_CHAN0, (uint8_t)((pit_counter & 0xFF00) >> 8));
}

void PIT_Reset() {
	Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_SQWAVE_GENERATOR | PIT_ACCESS_LOHIBYTE | PIT_CHANNEL_0);
	Outb(PIT_IOPORT_CHAN0, 0);
	Outb(PIT_IOPORT_CHAN0, 0);
}

uint16_t PIT_ReadCounter() {
	uint16_t counter = 0;
	Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_INT_TERM_COUNT | PIT_ACCESS_LATCH | PIT_CHANNEL_0);
	counter  =  Inb(PIT_IOPORT_CHAN0);
	counter |= (Inb(PIT_IOPORT_CHAN0) << 8);
	return counter;
}
