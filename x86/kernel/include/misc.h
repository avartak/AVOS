#ifndef X86_KERNEL_MISC_H
#define X86_KERNEL_MISC_H

#include <stdint.h>

#define DisableInterrupts() __asm__ __volatile__("cli" : : :);
#define EnableInterrupts()  __asm__ __volatile__("sti" : : :);
#define HaltSystem()        __asm__ __volatile__("hlt" : : :);
#define Int(__INTERRUPT__)  __asm__ __volatile__("int $"#__INTERRUPT__";" : : :)

#endif
