#ifndef KERNEL_MACHINE_H
#define KERNEL_MACHINE_H

#include <kernel/include/common.h>

#define System_Halt()                     __asm__ __volatile__("hlt" : : :);

#define Interrupt_DisableAll()            __asm__ __volatile__("cli" : : :);
#define Interrupt_EnableAll()             __asm__ __volatile__("sti" : : :);
#define Interrupt_Invoke(__INTERRUPT__)   __asm__ __volatile__("int $"#__INTERRUPT__";" : : :);

extern void Interrupt_End(uint8_t);
extern void Interrupt_Enable(uint8_t);
extern void Interrupt_Disable(uint8_t);

extern bool Memory_AllocateBlock(uintptr_t);
extern bool Memory_FreeBlock(uintptr_t);

struct Process_State {
    uint16_t attrib;
    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
    uint16_t ss;
    uint16_t ss0;
    uint32_t esp0;
    uint32_t eip;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;
    uint32_t cr3;
}__attribute__((packed));

#endif
