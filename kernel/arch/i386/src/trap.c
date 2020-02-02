#include <kernel/arch/i386/include/trap.h>
#include <kernel/arch/apic/include/pic.h>
#include <kernel/arch/timer/include/pit.h>
#include <kernel/arch/keyboard/include/keyboard.h>

void Interrupt_Handler(uint32_t interrupt) {

    if (interrupt == 0x20) PIT_HandleInterrupt();
    else if (interrupt == 0x21) Keyboard_HandleInterrupt();
}

void Interrupt_AddEntry(uint8_t entry, uintptr_t handler) {

        X86_IDT_SetupEntry(&(State_GetCPU()->idt[entry]), handler, X86_GDT_SEG_KERN_CODE, X86_IDT_FLAGS_PRESENT | X86_IDT_FLAGS_DPL0 | X86_IDT_TYPE_INTR32);
}
