#ifndef KERNEL_CORE_STACK_H
#define KERNEL_CORE_STACK_H

struct Stack {
	void*    base;
	void*    ptr;
	size_t   size;
	int32_t  flags; // Not sure this should not be uint32_t. But until I know (or think) something better I will leave it as per the POSIX specification
}__attribute__((packed));

#endif
