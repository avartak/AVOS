#ifndef X86_KERNEL_MISC_H
#define X86_KERNEL_MISC_H

#include <stdint.h>

#define DisableInterrupts() asm volatile("cli" : : :);
#define EnableInterrupts()  asm volatile("sti" : : :);
#define HaltSystem()        asm volatile("hlt" : : :);
#define Int(__INTERRUPT__)  asm volatile("int $"#__INTERRUPT__";" : : :)

#endif
