#ifndef KERNEL_MESSAGING_H
#define KERNEL_MESSAGING_H

struct Message {
	uint32_t source;
	uint32_t attrib;
	uint8_t  message[56];
}__attribute__((packed));

extern uint32_t Message_Send   (uint32_t dest, struct Message* msg, uint32_t flags);
extern uint32_t Message_Receive(uint32_t src , struct Message* msg, uint32_t flags);

#endif
