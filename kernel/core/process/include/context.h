#ifndef KERNEL_CORE_CONTEXT_H
#define KERNEL_CORE_CONTEXT_H

#include <stdint.h>

struct Context {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
}__attribute__((packed));

extern void Context_Switch(struct Context** old_context, struct Context* new_context);

#endif
