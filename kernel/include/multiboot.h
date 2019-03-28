#ifndef KERNEL_MULTIBOOT_H
#define KERNEL_MULTIBOOT_H

#include <kernel/include/common.h>

#define MULTIBOOT_TAG_TYPE_END  0
#define MULTIBOOT_TAG_TYPE_MMAP 6


struct Multiboot_Tag {
	uint32_t type;
	uint32_t size;
};

extern uint32_t* MultibootInfo;

#endif
