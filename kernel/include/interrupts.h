#ifndef X86_KERNEL_INTERRUPTS_H
#define X86_KERNEL_INTERRUPTS_H

#include <kernel/include/common.h>

struct Interrupt_Handler {
	struct Interrupt_Handler* next;
	uint32_t (*handler)(void);
	uint32_t id;
	uint32_t process;
};

extern int32_t   Interrupt_kernel_reentries;
extern uint16_t  Interrupt_active_IRQs;
extern uintptr_t Interrupt_stack;
extern struct    Interrupt_Handler* Interrupt_Handler_map;

extern void      Interrupt_Initialize();
extern void      Interrupt_Handle(uint32_t interrupt);
extern uint8_t   Interrupt_AddHandler(struct Interrupt_Handler* handler, uint8_t interrupt);
extern bool      Interrupt_RemoveHandler(uint32_t id, uint8_t interrupt);

#endif
