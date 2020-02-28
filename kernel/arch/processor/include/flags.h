#ifndef KERNEL_FLAGS_H
#define KERNEL_FLAGS_H

#include <stdint.h>
#include <stdbool.h>

#define EFLAGS_CF               1
#define EFLAGS_PF               4
#define EFLAGS_AF               0x10
#define EFLAGS_ZF               0x40
#define EFLAGS_SF               0x80
#define EFLAGS_TF               0x100
#define EFLAGS_IF               0x200
#define EFLAGS_DF               0x400
#define EFLAGS_OF               0x800
#define EFLAGS_IOPL             0x3000
#define EFLAGS_NT               0x4000
#define EFLAGS_RF               0x10000
#define EFLAGS_VM               0x20000
#define EFLAGS_AC               0x40000
#define EFLAGS_VIF              0x80000
#define EFLAGS_VIP              0x100000
#define EFLAGS_ID               0x200000

extern uint16_t ReadFlags();
extern uint32_t ReadEFlags();

extern bool InterruptsEnabled();

#endif
