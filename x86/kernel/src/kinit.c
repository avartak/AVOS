#include <x86/kernel/include/gdt.h>
#include <x86/kernel/include/tss.h>
#include <x86/kernel/include/idt.h>
#include <x86/kernel/include/physmem.h>
#include <x86/kernel/include/pic.h>
#include <x86/kernel/include/irqs.h>
#include <x86/kernel/include/welcome.h>
#include <kernel/include/heap.h>

void Kinit(uint32_t* boot_info) {

    GDT_Initialize();

    TSS_Initialize();

    IDT_Initialize();

    Physical_Memory_Initialize(boot_info);

    Heap_Initialize();

    PIC_Initialize();
   
    IRQ_Initialize();
   
    Welcome();

}
