#ifndef KERNEL_MESSAGING_H
#define KERNEL_MESSAGING_H

#include <kernel/include/common.h>
#include <kernel/include/process.h>

#define MESSAGE_SENDING   0x1
#define MESSAGE_RECEIVING 0x2

struct Message {
	uint32_t source;
	uint32_t attrib;
	uint8_t  message[56];
}__attribute__((packed));

#endif
