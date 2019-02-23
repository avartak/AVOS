#include <x86/drivers/include/pic.h>
#include <kernel/include/interrupts.h>
#include <kernel/include/dispensary.h>

#include <stddef.h>

struct Interrupt_Handler* Interrupt_Handler_map = MEMORY_NULL_PTR;

void Interrupt_Initialize() {
	Interrupt_Handler_map = (struct Interrupt_Handler*)(Memory_NodeDispenser_New());
	for (size_t i = 0; i < MEMORY_SIZE_PAGE/sizeof(struct Interrupt_Handler); i++) {
		Interrupt_Handler_map[i].next     = MEMORY_NULL_PTR;
		Interrupt_Handler_map[i].handler  = MEMORY_NULL_PTR;
		Interrupt_Handler_map[i].id       = 0;
		Interrupt_Handler_map[i].process  = 0;
	} 
}

uint8_t Interrupt_AddHandler(struct Interrupt_Handler* handler, uint8_t interrupt) {
	if (handler == MEMORY_NULL_PTR || handler->handler == MEMORY_NULL_PTR) return 0;
	struct Interrupt_Handler* current_handler = &(Interrupt_Handler_map[interrupt]);
	if (current_handler->handler == MEMORY_NULL_PTR) {
		current_handler->handler = handler->handler;
		return 1;
	}
	while (current_handler->next != MEMORY_NULL_PTR) current_handler = current_handler->next;
	current_handler->next = handler;
	return 2;
}

bool Interrupt_RemoveHandler(uint32_t id, uint8_t interrupt) {
    struct Interrupt_Handler* current_handler = &(Interrupt_Handler_map[interrupt]);
	if (current_handler->id == id) {
		if (current_handler->next == MEMORY_NULL_PTR) return false;
		else {
			current_handler = current_handler->next;
			return true;
		}
	}
    while (current_handler->next != MEMORY_NULL_PTR) {
		if (current_handler->next->id == id) {
			current_handler->next = current_handler->next->next;
			return true;
		}
		current_handler = current_handler->next;
	}
	return false;
}

void Interrupt_Handle(uint32_t interrupt) {

	struct Interrupt_Handler* handler = &(Interrupt_Handler_map[interrupt]);
	while (true) {
		if (handler->handler == MEMORY_NULL_PTR || (handler->handler)() == 2 || handler->next == MEMORY_NULL_PTR) break;
		handler = handler->next;
	} 

    if (interrupt >= PIC_IRQ_OFFSET && interrupt < PIC_IRQ_OFFSET + PIC_NUM_IRQS) {
        PIC_SendEOI(interrupt - PIC_IRQ_OFFSET);
    }

    return;

}

