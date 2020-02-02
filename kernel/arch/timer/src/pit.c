#include <kernel/arch/timer/include/pit.h>
#include <kernel/arch/i386/include/ioports.h>
#include <kernel/arch/i386/include/trap.h>
#include <kernel/arch/apic/include/apic.h>
#include <kernel/arch/apic/include/ioapic.h>
#include <kernel/arch/apic/include/lapic.h>
#include <stdatomic.h>

uint64_t PIT_ticks = 0;
bool     PIT_enabled = false;

void     PIT_Initialize();
void     PIT_Reset();
uint16_t PIT_ReadCounter();
void     PIT_HandleInterrupt();
void     PIT_Delay(uint32_t delay);

void PIT_Initialize() {
    uint32_t pit_counter = PIT_BASE_FREQUENCY/PIT_TARGET_FREQUENCY;

    X86_Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_RATE_GENERATOR | PIT_ACCESS_LOHIBYTE | PIT_CHANNEL_0);
    X86_Outb(PIT_IOPORT_CHAN0, (uint8_t) (pit_counter & 0x00FF));
    X86_Outb(PIT_IOPORT_CHAN0, (uint8_t)((pit_counter & 0xFF00) >> 8));

	TRAP_ADDINTERRUPT(0x20);
    IOAPIC_EnableInterrupt(0, 0x20, LocalAPIC_ID());
	PIT_enabled = true;
}

void PIT_Reset() {
    X86_Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_RATE_GENERATOR | PIT_ACCESS_LOHIBYTE | PIT_CHANNEL_0);
    X86_Outb(PIT_IOPORT_CHAN0, 0);
    X86_Outb(PIT_IOPORT_CHAN0, 0);

	PIT_ticks = 0;
}

uint16_t PIT_ReadCounter() {
    uint16_t counter = 0;
    X86_Outb(PIT_IOPORT_COMD , PIT_COUNTMODE_BIN | PIT_OPERMODE_INT_TERM_COUNT | PIT_ACCESS_LATCH | PIT_CHANNEL_0);
    counter  =  X86_Inb(PIT_IOPORT_CHAN0);
    counter |= (X86_Inb(PIT_IOPORT_CHAN0) << 8);
    return counter;
}

void PIT_HandleInterrupt() {
	PIT_ticks++;
}

void PIT_Delay(uint32_t delay) {

	uint64_t iticks = PIT_ticks;

	atomic_signal_fence(memory_order_seq_cst);

	while (PIT_ticks - iticks <= delay);

}
