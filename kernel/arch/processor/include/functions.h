#ifndef KERNEL_X86_FUNCTIONS_H
#define KERNEL_X86_FUNCTIONS_H

extern void      X86_Halt();
extern void      X86_DisableInterrupts();
extern void      X86_EnableInterrupts();
extern uintptr_t X86_GetStackBase();
extern void      X86_SwitchToHigherHalf();

#endif
