#include <kernel/include/ioports.h>

bool IOPort_EnableReadAccess(struct IOPort_Node* node) {
	if (node == MEMORY_NULL_PTR) return false;
	node->attrib |= (uint16_t)(IOPORT_READ_ACCESS_BIT);	
	return true;
}

bool IOPort_EnableWriteAccess(struct IOPort_Node* node) {
	if (node == MEMORY_NULL_PTR) return false;
	node->attrib |= (uint16_t)(IOPORT_WRITE_ACCESS_BIT);	
	return true;
}

bool IOPort_DisableReadAccess(struct IOPort_Node* node) {
	if (node == MEMORY_NULL_PTR) return false;
	node->attrib &= (uint16_t)(~IOPORT_READ_ACCESS_BIT);	
	return true;
}

bool IOPort_DisableWriteAccess(struct IOPort_Node* node) {
	if (node == MEMORY_NULL_PTR) return false;
	node->attrib &= (uint16_t)(~IOPORT_WRITE_ACCESS_BIT);	
	return true;
}

bool IOPort_Add(struct IOPort_Node* list, struct IOPort_Node* node) {
	if (list == MEMORY_NULL_PTR) return false;
	while (list->next != MEMORY_NULL_PTR) {
		if (list->port == node->port) return false;
		else list = list->next;
	}
	list->next = node;
	return true;	
}

struct IOPort_Node* IOPort_Remove(struct IOPort_Node* list, uint16_t port) {
	if (list == MEMORY_NULL_PTR) return MEMORY_NULL_PTR;
	struct IOPort_Node* ret_port = MEMORY_NULL_PTR;
	if (list->port == port) {
		ret_port = list;
		list = list->next;
	}
	else {
		while (list->next != MEMORY_NULL_PTR) {
			if (list->next->port == port) {
				ret_port = list->next;
				list->next = list->next->next;
			}
		}
	}
	return ret_port;
}
