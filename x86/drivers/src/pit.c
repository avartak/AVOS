#include <x86/drivers/include/pit.h>
#include <x86/drivers/include/pic.h>
#include <x86/drivers/include/io.h>
#include <x86/kernel/include/interrupts.h>
#include <kernel/include/clock.h>

void PIT_Initialize(uint32_t tick_freq) {
	uint32_t pit_counter = PIT_BASE_FREQUENCY/tick_freq;

	Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_SQWAVE_GENERATOR | PIT_ACCESS_LOHIBYTE | PIT_CHANNEL_0);
	Outb(PIT_IOPORT_CHAN0, (uint8_t) (pit_counter & 0x00FF));
	Outb(PIT_IOPORT_CHAN0, (uint8_t)((pit_counter & 0xFF00) >> 8));

	PIC_DisableInterrupt(PIT_IRQLINE);
	Interrupt_Handler_map[PIT_IRQLINE+PIC_IRQ_OFFSET].handler = &Clock_HandleInterrupt;
	PIC_EnableInterrupt(PIT_IRQLINE);
}

void PIT_Reset() {
	Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_SQWAVE_GENERATOR | PIT_ACCESS_LOHIBYTE | PIT_CHANNEL_0);
	Outb(PIT_IOPORT_CHAN0, 0);
	Outb(PIT_IOPORT_CHAN0, 0);
}

uint16_t Read_Clock_Counter() {
	uint16_t clock_counter = 0;
	Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_INT_TERM_COUNT | PIT_ACCESS_LATCH | PIT_CHANNEL_0);
	clock_counter  =  Inb(PIT_IOPORT_CHAN0);
	clock_counter |= (Inb(PIT_IOPORT_CHAN0) << 8);
	return clock_counter;
}
