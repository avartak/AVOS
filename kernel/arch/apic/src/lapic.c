#include <kernel/arch/apic/include/apic.h>
#include <kernel/arch/apic/include/lapic.h>
#include <kernel/arch/tasking/include/cpu.h>
#include <kernel/arch/tasking/include/context.h>
#include <kernel/arch/tasking/include/interrupt.h>
#include <kernel/devices/pit/include/pit.h>
#include <kernel/devices/console/include/console.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/taskmaster/include/process.h>
#include <kernel/core/timer/include/timer.h>

uintptr_t LocalAPIC_address = 0;
size_t    LocalAPIC_Num = 0;
uintptr_t LocalAPIC_InfoPtrs[MACHINE_MAX_CPUS];

uint32_t LocalAPIC_WriteTo(size_t index, uint32_t value) {
	
	volatile uint32_t* lapic = (volatile uint32_t*)LocalAPIC_address;
	lapic[index] = value;
	return lapic[LAPIC_REG_ID];
}

uint32_t LocalAPIC_ReadFrom(size_t index) {

    volatile uint32_t* lapic = (volatile uint32_t*)LocalAPIC_address;
    return lapic[index];

}

void LocalAPIC_EOI() {

	LocalAPIC_WriteTo(LAPIC_REG_EOI, 0);	 

}

uint8_t LocalAPIC_ID() {
	return LocalAPIC_ReadFrom(LAPIC_REG_ID) >> 24;
}

bool LocalAPIC_Initialize() {

	APIC_SaveInfo();

	// Enable the local APIC by writing to the spurious interrupt vector register
	LocalAPIC_WriteTo(LAPIC_REG_SIVR, LAPIC_SIVR_SOFT_ENABLE | LAPIC_SIVR_INTR_SPURIOUS);

	// Disable the timer, LINT0, LINT1, ERROR local interrupts
	LocalAPIC_WriteTo(LAPIC_REG_TIMER, LAPIC_LVT_MASKED);	 
	LocalAPIC_WriteTo(LAPIC_REG_LINT0, LAPIC_LVT_MASKED);	 
	LocalAPIC_WriteTo(LAPIC_REG_LINT1, LAPIC_LVT_MASKED);	 
	LocalAPIC_WriteTo(LAPIC_REG_ERROR, LAPIC_LVT_MASKED);	 

	// Disable local interrupts from the thermal sensor and performance monitors (if they exist)
	if (((LocalAPIC_ReadFrom(LAPIC_REG_VERSION) >> 16) & 0xFF) > 3) LocalAPIC_WriteTo(LAPIC_REG_PCMR, LAPIC_LVT_MASKED);
	if (((LocalAPIC_ReadFrom(LAPIC_REG_VERSION) >> 16) & 0xFF) > 4) LocalAPIC_WriteTo(LAPIC_REG_TSR , LAPIC_LVT_MASKED);

	// Clear registers used for configuring the local timer
	LocalAPIC_WriteTo(LAPIC_REG_TICR, 0);	 
	LocalAPIC_WriteTo(LAPIC_REG_TDCR, 0);	 

	// Clear the error state register - needs writing twice, once for arming and then for the actual write
	LocalAPIC_WriteTo(LAPIC_REG_ESR, 0);	 
	LocalAPIC_WriteTo(LAPIC_REG_ESR, 0);	 

	// Clear pending interrupts
	LocalAPIC_WriteTo(LAPIC_REG_EOI, 0);	 

	// Broadcast an INIT-LEVEL-DEASSERT to reset the arbitration priority register
	LocalAPIC_WriteTo(LAPIC_REG_ICRHI, 0);	 
	LocalAPIC_WriteTo(LAPIC_REG_ICRLO, LAPIC_ICR_DEST_ALLSELF | LAPIC_LVT_DELIVERY_INIT | LAPIC_ICR_TRIGGER_LEVEL | LAPIC_ICR_LEVEL_DEASSERT);

	// Make sure the broadcast went through
	while (LocalAPIC_ReadFrom(LAPIC_REG_ICRLO) & LAPIC_ICR_DELIVERY_PENDING);

	// Enable all interrupts in the local APIC (this does not apply to the CPU) by setting the task priority register to the lowest possible value i.e. 0
	LocalAPIC_WriteTo(LAPIC_REG_TPR, 0); 

	return true;
}

size_t LocalAPIC_GetTimerFrequency() {

	size_t iter = 10;
	size_t freq = 0;
	for (size_t i = 0; i < iter; i++) {
		
		// Tell APIC timer to use divider 16
		LocalAPIC_WriteTo(LAPIC_REG_TDCR, LAPIC_TDCR_DIVIDE_BY_16);
		
		// Setup the APIC timer
		LocalAPIC_WriteTo(LAPIC_REG_TIMER, LAPIC_LVT_TIMER_MODE_PERIODIC | 0xF0);
		
		// Set APIC init counter to 0xFFFFFFFF (large value)
		LocalAPIC_WriteTo(LAPIC_REG_TICR, 0xFFFFFFFF);
		
		// Wait for the PIT to count 10 ms
		PIT_Delay(100);
		
		// Stop and reset the APIC timer
		LocalAPIC_WriteTo(LAPIC_REG_TIMER, LAPIC_LVT_MASKED);
		
		// Now we know how often the APIC timer has ticked in 10 ms
		freq += 0xFFFFFFFF - LocalAPIC_ReadFrom(LAPIC_REG_TCCR);
	
	}

	// The timer frequency
	freq /= iter;
	freq /= 10;
	freq = freq << 4;

	Console_Print("CPU %d timer frequency : %u\n", LocalAPIC_ID(), freq);

	return freq;

}

void LocalAPIC_Initialize_Timer(uint8_t vector, size_t freq) {

	LocalAPIC_WriteTo(LAPIC_REG_TIMER, LAPIC_LVT_MASKED);

	size_t lapic_freq = LocalAPIC_GetTimerFrequency();
	if (lapic_freq == 0) return;

	SpinLock_Acquire(&State_lock);
	STATE_CURRENT->kernel_task->timer.ticks = 0;
	STATE_CURRENT->kernel_task->timer.seq_counter = 0;
	SpinLock_Release(&State_lock);
	Interrupt_AddHandler(vector, LocalAPIC_Timer_HandleInterrupt);

    LocalAPIC_WriteTo(LAPIC_REG_TDCR, LAPIC_TDCR_DIVIDE_BY_16);
    LocalAPIC_WriteTo(LAPIC_REG_TIMER, LAPIC_LVT_TIMER_MODE_PERIODIC | vector);
    LocalAPIC_WriteTo(LAPIC_REG_TICR, (lapic_freq/freq)*1000);
}

void LocalAPIC_Timer_HandleInterrupt(__attribute__((unused))struct IContext* frame) {
	Timer_Increment(&(STATE_CURRENT->kernel_task->timer));
	LocalAPIC_EOI();
}

