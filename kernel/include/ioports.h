#ifndef KERNEL_IOPORTS_H
#define KERNEL_IOPORTS_H

#include <kernel/include/common.h>

#define IOPORT_READ_ACCESS_BIT    1
#define IOPORT_WRITE_ACCESS_BIT   2

struct IOPort_Node {
	struct IOPort_Node* next;
	uint16_t port;
	uint16_t attrib;
};

extern                bool IOPort_EnableReadAccess  (struct IOPort_Node* node);
extern                bool IOPort_EnableWriteAccess (struct IOPort_Node* node);
extern                bool IOPort_DisableReadAccess (struct IOPort_Node* node);
extern                bool IOPort_DisableWriteAccess(struct IOPort_Node* node);
extern                bool IOPort_Add   (struct IOPort_Node* list, struct IOPort_Node* node);
extern struct IOPort_Node* IOPort_Remove(struct IOPort_Node* list, uint16_t port);

#endif
