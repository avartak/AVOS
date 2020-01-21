#ifndef KERNEL_X86_PSW_H
#define KERNEL_X86_PSW_H

#include <stdint.h>

#define X86_EFLAGS_CF               1
#define X86_EFLAGS_PF               4
#define X86_EFLAGS_AF               0x10
#define X86_EFLAGS_ZF               0x40
#define X86_EFLAGS_SF               0x80
#define X86_EFLAGS_TF               0x100
#define X86_EFLAGS_IF               0x200
#define X86_EFLAGS_DF               0x400
#define X86_EFLAGS_OF               0x800
#define X86_EFLAGS_IOPL             0x3000
#define X86_EFLAGS_NT               0x4000
#define X86_EFLAGS_RF               0x10000
#define X86_EFLAGS_VM               0x20000
#define X86_EFLAGS_AC               0x40000
#define X86_EFLAGS_VIF              0x80000
#define X86_EFLAGS_VIP              0x100000
#define X86_EFLAGS_ID               0x200000

#endif
