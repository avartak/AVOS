#include <kernel/devices/timer/include/pit.h>
#include <kernel/arch/processor/include/ioports.h>
#include <kernel/arch/apic/include/apic.h>
#include <kernel/arch/apic/include/ioapic.h>
#include <kernel/arch/apic/include/lapic.h>
#include <stdatomic.h>

uint64_t PIT_ticks = 0;
bool     PIT_enabled = false;

struct SpinLock PIT_lock;

void PIT_Initialize(uint8_t irq, uint8_t vector) {
	Interrupt_AddHandler(vector, PIT_HandleInterrupt);
    IOAPIC_EnableInterrupt(irq, vector, LocalAPIC_ID());

	SpinLock_Initialize(&PIT_lock);

	PIT_enabled = true;
}

void PIT_Set(size_t freq) {

	SpinLock_Acquire(&PIT_lock);

	PIT_ticks = 0;
    uint32_t pit_counter = PIT_BASE_FREQUENCY/freq;

    X86_Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_RATE_GENERATOR | PIT_ACCESS_LOHIBYTE | PIT_CHANNEL_0);
    X86_Outb(PIT_IOPORT_CHAN0, (uint8_t) (pit_counter & 0x00FF));
    X86_Outb(PIT_IOPORT_CHAN0, (uint8_t)((pit_counter & 0xFF00) >> 8));
}

void PIT_Reset() {
    X86_Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_RATE_GENERATOR | PIT_ACCESS_LOHIBYTE | PIT_CHANNEL_0);
    X86_Outb(PIT_IOPORT_CHAN0, 0);
    X86_Outb(PIT_IOPORT_CHAN0, 0);

	SpinLock_Release(&PIT_lock);
}

uint16_t PIT_ReadCounter() {
    uint16_t counter = 0;
    X86_Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_INT_TERM_COUNT | PIT_ACCESS_LATCH | PIT_CHANNEL_0);
    counter  =  X86_Inb(PIT_IOPORT_CHAN0);
    counter |= (X86_Inb(PIT_IOPORT_CHAN0) << 8);
    return counter;
}

void PIT_HandleInterrupt(__attribute__((unused))struct Interrupt_Frame* frame) {
	PIT_ticks++;
}

void PIT_Delay(uint32_t delay) {

	uint64_t iticks = PIT_ticks;

	atomic_signal_fence(memory_order_seq_cst);

	while (PIT_ticks - iticks <= delay);
}
