#ifndef KERNEL_MACHINE_H
#define KERNEL_MACHINE_H

#include <stdint.h>
#include <stdbool.h>

#define System_Halt()                     __asm__ __volatile__("hlt" : : :);

#define Interrupt_DisableAll()            __asm__ __volatile__("cli" : : :);
#define Interrupt_EnableAll()             __asm__ __volatile__("sti" : : :);
#define Interrupt_Invoke(__INTERRUPT__)   __asm__ __volatile__("int $"#__INTERRUPT__";" : : :);

extern void Interrupt_End(uint8_t);
extern void Interrupt_Enable(uint8_t);
extern void Interrupt_Disable(uint8_t);

extern bool Memory_AllocateBlock(uintptr_t);
extern bool Memory_FreeBlock(uintptr_t);

#endif
