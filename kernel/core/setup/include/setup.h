#ifndef KERNEL_INITIAL_SETUP_H
#define KERNEL_INITIAL_SETUP_H

#include <stdint.h>
#include <stdatomic.h>

#define KERNEL_HIGHER_HALF_OFFSET 0x80000000
#define KERNEL_MMAP_VIRTUAL_START 0x80000000
#define KERNEL_MMAP_VIRTUAL_END   0xF0000000
#define KERNEL_MMAP_LOWMEM_START  0x80000000
#define KERNEL_MMAP_LOWMEM_END    0x80100000
#define KERNEL_MMAP_HIMEMIO_START 0xF0000000
#define KERNEL_MMAP_START_ADDR    0x80100000

#define KERNEL_PAGE_SIZE_IN_BITS  12
#define KERNEL_PAGE_SIZE_IN_BYTES (1 << 12)
#define KERNEL_BYTE_IN_BITS       3

#define KERNEL_STACK_ALIGNMENT    (1 << 4)
#define KERNEL_STACK_SIZE         0x2000
#define KERNEL_AP_BOOT_START_ADDR 0x8000
#define KERNEL_AP_BOOT_START_SIZE 0x1000

#define KERNEL_MAX_PROCS          0x40

#define KERNEL_NUM_SYSCALLS       0x100
#define KERNEL_NUM_SIGNALS        0x20


#define MACHINE_MAX_CPUS          0xFF
#define MACHINE_MAX_IOAPICS       0xFF

#define PHYSADDR(x)               ((uintptr_t)x-KERNEL_HIGHER_HALF_OFFSET)
#define KERNADDR(x)               ((uintptr_t)x+KERNEL_HIGHER_HALF_OFFSET)

typedef uint32_t pid_t;
typedef uint32_t sigset_t;
typedef uint64_t clock_t;
typedef _Atomic uint64_t atomic_clock_t;

#endif
