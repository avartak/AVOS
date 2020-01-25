#ifndef KERNEL_X86_FUNCTIONS_H
#define KERNEL_X86_FUNCTIONS_H

extern void    X86_Halt();
extern uint8_t X86_DisableInterrupts();
extern void    X86_EnableInterrupts();
extern void    X86_RestoreInterrupts(uint8_t intr_priority);

#endif
