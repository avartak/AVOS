#ifndef KERNEL_FUNCTIONS_H
#define KERNEL_FUNCTIONS_H

extern void      Halt();
extern void      DisableInterrupts();
extern void      EnableInterrupts();
extern uintptr_t GetStackBase();

#endif
