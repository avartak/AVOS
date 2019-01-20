#include <x86/kernel/include/paging.h>

#include <stdint.h>
#include <stddef.h>

uint32_t page_directory[1024]__attribute__((aligned(4096)));
uint32_t kern_pagetable[1024]__attribute__((aligned(4096)));
uint32_t pmap_pagetable[1023]__attribute__((aligned(4096)));

void InitPaging() {

	for (size_t i = 0; i < 1024; i++) page_directory[i] = 0;	

	for (size_t i = 0; i < 1024; i++) { 
    	kern_pagetable[i] = (i * 0x1000) | 3;
	}		
    page_directory[768] = (((uint32_t)kern_pagetable) | 3) - LOC_KERNEL_HH_OFFSET;

    for(size_t i = 0; i < 1024; i++) {
        pmap_pagetable[i] = (i * 0x1000 + LOC_PAGEMAP_PM) | 3;
    }
    page_directory[769] = (((uint32_t)pmap_pagetable) | 3) - LOC_KERNEL_HH_OFFSET;

	LoadPageDirectory((uint32_t)page_directory - LOC_KERNEL_HH_OFFSET);
	EnablePGBitInCR0();

}
