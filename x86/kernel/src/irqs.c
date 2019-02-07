#include <x86/kernel/include/pic.h>
#include <x86/kernel/include/irqs.h>

void IRQ_Initialize() {

	PIC_EnableInterrupt(1);

}
