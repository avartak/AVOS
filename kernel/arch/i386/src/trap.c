#include <kernel/arch/i386/include/trap.h>
#include <kernel/arch/apic/include/pic.h>
#include <kernel/arch/timer/include/pit.h>
#include <kernel/arch/keyboard/include/keyboard.h>

void Interrupt_Handler(uint32_t interrupt) {

    if (interrupt == 0x20) PIT_HandleInterrupt();
    if (interrupt == 0x21) Keyboard_HandleInterrupt();

}
