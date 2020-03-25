#ifndef KERNEL_PHYSMEM_H
#define KERNEL_PHYSMEM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kernel/core/setup/include/setup.h>

#define PAGE_MAP_IDX(addr, order)         (  addr >> (KERNEL_PAGE_SIZE_IN_BITS+1+KERNEL_BYTE_IN_BITS+order) )
#define PAGE_MAP_BIT(addr, order)         ( (addr >> (KERNEL_PAGE_SIZE_IN_BITS+1+order))  & ((1 << KERNEL_BYTE_IN_BITS)-1) )

#define PAGE_MAP_ISBITSET(addr, order)    ( (Page_maps[order][PAGE_MAP_IDX(addr, order)]  &  (1 << PAGE_MAP_BIT(addr, order))) > 0 )
#define PAGE_MAP_SETBIT(addr, order)      (  Page_maps[order][PAGE_MAP_IDX(addr, order)] |=  (1 << PAGE_MAP_BIT(addr, order)) )
#define PAGE_MAP_CLEARBIT(addr, order)    (  Page_maps[order][PAGE_MAP_IDX(addr, order)] &= ~(1 << PAGE_MAP_BIT(addr, order)) )
#define PAGE_MAP_TOGGLEBIT(addr, order)   (  Page_maps[order][PAGE_MAP_IDX(addr, order)] ^=  (1 << PAGE_MAP_BIT(addr, order)) )

#define PAGE_BUDDY(addr, order)           (  addr % (1 << (KERNEL_PAGE_SIZE_IN_BITS+1+order)) == 0 ? addr + (1 << (KERNEL_PAGE_SIZE_IN_BITS+order)) : addr - (1 << (KERNEL_PAGE_SIZE_IN_BITS+order)) )

#define PAGE_SIZE_AT_ORDER(order)         (size_t)(1 << (KERNEL_PAGE_SIZE_IN_BITS+order))
#define PAGE_LIST_NULL                    ((struct Page_List*)0xFFFFFFFF)
#define PAGE_MAX_ORDER                    10

struct Page_List {
	struct Page_List* next;
	struct Page_List* prev;
};

extern uint8_t* Page_maps[];
extern size_t Page_maps_size[];
extern struct Page_List* Page_lists[];
extern struct IRQLock Page_operation_lock;

extern void  Page_BuddyMaps_Initialize();
extern void  Page_MapMemoryChunk(uint64_t chunk_base, uint64_t chunk_size);
extern void* Page_Acquire(uint8_t order);
extern void  Page_Release(void* pointer, uint8_t order);

#endif
