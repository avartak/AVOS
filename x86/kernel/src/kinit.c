#include <x86/kernel/include/gdt.h>
#include <x86/kernel/include/tss.h>
#include <x86/kernel/include/idt.h>
#include <x86/kernel/include/physmem.h>
#include <x86/kernel/include/pic.h>
#include <x86/kernel/include/irqs.h>
#include <x86/kernel/include/welcome.h>

void Kinit(uint32_t* boot_info) {

    GDT_Initialize();

    TSS_Initialize();

    IDT_Initialize();

    Memory_Initialize(boot_info);

    PIC_Initialize();
   
    IRQ_Initialize();
   
    Welcome();

}
