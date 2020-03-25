#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct Process;
struct IContext;

extern void (*Interrupt_Handlers[])(struct IContext*);

extern void      Interrupt_Return(struct IContext* frame);
extern void      Interrupt_Handle(struct IContext* frame);
extern void      Interrupt_AddHandler(uint8_t entry, void (*handler)(struct IContext*));

extern void      Interrupt_0x20();
extern void      Interrupt_0x21();
extern void      Interrupt_0x30();
extern void      Interrupt_0x80();

#endif
