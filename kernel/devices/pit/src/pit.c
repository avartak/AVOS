#include <kernel/devices/pit/include/pit.h>
#include <kernel/arch/tasking/include/interrupt.h>
#include <kernel/arch/processor/include/ioports.h>
#include <kernel/arch/apic/include/apic.h>
#include <kernel/arch/apic/include/ioapic.h>
#include <kernel/arch/apic/include/lapic.h>
#include <stdatomic.h>

struct Timer PIT_timer = {0, 0};

void PIT_Initialize(uint8_t irq, uint8_t vector) {
	Interrupt_AddHandler(vector, PIT_HandleInterrupt);
    IOAPIC_EnableInterrupt(irq, vector, LocalAPIC_ID());
	PIT_Set(10000);
}

void PIT_Set(size_t freq) {

	PIT_timer.seq_counter = 0;
	PIT_timer.ticks = 0;
    uint32_t pit_counter = PIT_BASE_FREQUENCY/freq;

    Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_SQWAVE_GENERATOR | PIT_ACCESS_LOHIBYTE | PIT_CHANNEL_0);
    Outb(PIT_IOPORT_CHAN0, (uint8_t) (pit_counter & 0x00FF));
    Outb(PIT_IOPORT_CHAN0, (uint8_t)((pit_counter & 0xFF00) >> 8));
}

void PIT_Reset() {

    Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_SQWAVE_GENERATOR | PIT_ACCESS_LOHIBYTE | PIT_CHANNEL_0);
    Outb(PIT_IOPORT_CHAN0, 0);
    Outb(PIT_IOPORT_CHAN0, 0);
}

void PIT_HandleInterrupt(__attribute__((unused))struct IContext* frame) {

	Timer_Increment(&PIT_timer);
	LocalAPIC_EOI();
}

void PIT_Delay(uint32_t delay) {

	clock_t iticks = Timer_GetTicks(&PIT_timer);
	while (Timer_GetTicks(&PIT_timer) - iticks <= delay);
}
