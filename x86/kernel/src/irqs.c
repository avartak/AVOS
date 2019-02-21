#include <x86/drivers/include/pic.h>
#include <x86/kernel/include/irqs.h>

void IRQ_Initialize() {

	PIC_EnableInterrupt(0);
	PIC_EnableInterrupt(1);

}
