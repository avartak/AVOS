#ifndef X86_KERNEL_INTERRUPTS_H
#define X86_KERNEL_INTERRUPTS_H

#include <stdint.h>

struct Interrupt_TaskState {
	uint32_t eflags;
	uint32_t cs;
	uint32_t eip;
	uint32_t error_code;
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t ss;
	uint32_t cr3;
}__attribute__((packed));

extern void Interrupt_Handler(uint32_t interrupt);
extern void IRQTest(const char* str, uint8_t color);

extern void Interrupt_0 ();
extern void Interrupt_1 ();
extern void Interrupt_2 ();
extern void Interrupt_3 ();
extern void Interrupt_4 ();
extern void Interrupt_5 ();
extern void Interrupt_6 ();
extern void Interrupt_7 ();
extern void Interrupt_8 ();
extern void Interrupt_9 ();
extern void Interrupt_A ();
extern void Interrupt_B ();
extern void Interrupt_C ();
extern void Interrupt_D ();
extern void Interrupt_E ();
extern void Interrupt_F ();
extern void Interrupt_10();
extern void Interrupt_11();
extern void Interrupt_12();
extern void Interrupt_13();
extern void Interrupt_14();
extern void Interrupt_15();
extern void Interrupt_16();
extern void Interrupt_17();
extern void Interrupt_18();
extern void Interrupt_19();
extern void Interrupt_1A();
extern void Interrupt_1B();
extern void Interrupt_1C();
extern void Interrupt_1D();
extern void Interrupt_1E();
extern void Interrupt_1F();

extern void Interrupt_20();
extern void Interrupt_21();
extern void Interrupt_22();
extern void Interrupt_23();
extern void Interrupt_24();
extern void Interrupt_25();
extern void Interrupt_26();
extern void Interrupt_27();
extern void Interrupt_28();
extern void Interrupt_29();
extern void Interrupt_2A();
extern void Interrupt_2B();
extern void Interrupt_2C();
extern void Interrupt_2D();
extern void Interrupt_2E();
extern void Interrupt_2F();

extern void Interrupt_80();

#endif
