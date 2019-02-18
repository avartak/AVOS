#include <kernel/include/heap.h>

struct          Memory_Stack KHeap_free;
struct          Memory_Stack KHeap_inuse;

extern struct   Memory_NodeDispenser* Kernel_node_dispenser;
extern bool     Physical_Memory_AllocatePage(uintptr_t virtual_address);
extern bool     Physical_Memory_FreePage(uintptr_t virtual_address);

uintptr_t KHeap_Allocate(size_t nbytes) {
    struct Memory_Node* node = Memory_Stack_Extract(&KHeap_free, nbytes);
    if (node == MEMORY_NULL_PTR || node->pointer == (uintptr_t)MEMORY_NULL_PTR) return (uintptr_t)MEMORY_NULL_PTR;

	uintptr_t start_page  = (node->pointer) & PAGE_MASK;
	uintptr_t start_alloc = start_page + MEMORY_SIZE_PAGE;
	if (start_page == node->pointer) start_alloc = start_page;
	uintptr_t end_alloc = (node->pointer + nbytes - 1) & PAGE_MASK;

	bool check_alloc = true;
	uintptr_t unalloc = start_alloc;
	for (uintptr_t page = start_alloc; page <= end_alloc; page += MEMORY_SIZE_PAGE) {
		check_alloc = Physical_Memory_AllocatePage(page);
		if (!check_alloc) {
			unalloc = page;
			break;
		}
	}
	if (!check_alloc) {
		for (uintptr_t page = start_alloc; page <= end_alloc && page <= unalloc; page += MEMORY_SIZE_PAGE) Physical_Memory_FreePage(page);
		Memory_Stack_Insert(&KHeap_free, node, true);
		return (uintptr_t)MEMORY_NULL_PTR;
	}

	Memory_Stack_Insert(&KHeap_inuse, node, false);

    return node->pointer;
}

bool KHeap_Free(uintptr_t pointer) {
    struct Memory_Node* node = Memory_Stack_Get(&KHeap_inuse, pointer);
	if (node == MEMORY_NULL_PTR) return false;

	uintptr_t start_page = pointer & PAGE_MASK;
	uintptr_t end_page   = (node->pointer + node->size * Memory_Node_GetBaseSize(node->attrib) - 1) & PAGE_MASK;
	
	node->attrib = KHeap_free.attrib;
	node->next   = MEMORY_NULL_PTR;
	bool ret_val = Memory_Stack_Insert(&KHeap_free, node, true);
	if (!ret_val) {
		node->attrib = KHeap_inuse.attrib;
    	Memory_Stack_Insert(&KHeap_inuse, node, false);
		return ret_val; 
	}

	for (uintptr_t page = start_page; page <= end_page; page += MEMORY_SIZE_PAGE) {
		if (Memory_Stack_Contains(&KHeap_free, page, page + MEMORY_SIZE_PAGE - 1)) Physical_Memory_FreePage(page);
	}
	return true;
}

void KHeap_Initialize(uintptr_t vm_start, uintptr_t vm_end) {
    struct Memory_Node* free_node = Memory_NodeDispenser_Dispense(Kernel_node_dispenser);
    KHeap_free.start   = free_node;
    KHeap_free.size    = vm_end - vm_start;
    KHeap_free.attrib  = MEMORY_1B << 8;
    KHeap_free.node_dispenser = Kernel_node_dispenser;
   
    free_node->attrib  = KHeap_free.attrib;
    free_node->pointer = vm_start;
    free_node->size    = KHeap_free.size;
    free_node->next    = MEMORY_NULL_PTR;

    free_node = Memory_NodeDispenser_Dispense(Kernel_node_dispenser);
    KHeap_inuse.start  = MEMORY_NULL_PTR;
    KHeap_inuse.size   = 0;
    KHeap_inuse.attrib = MEMORY_1B << 8;
    KHeap_inuse.node_dispenser = Kernel_node_dispenser;
}
