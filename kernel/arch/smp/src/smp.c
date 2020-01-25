#include <kernel/arch/smp/include/smp.h>
#include <kernel/arch/apic/include/apic.h>
#include <kernel/arch/i386/include/ioports.h>
#include <kernel/arch/timer/include/pit.h>

bool SMP_Initialize_CPU(uint8_t local_apic_id, uint32_t boot_address) {

    X86_Outb(0x70, 0xF);
    X86_Outb(0x71, 0xA);

    uint16_t* warm_reset_vector = (uint16_t*)((0x40<<4 | 0x67) + KERNEL_HIGHER_HALF_OFFSET);
    warm_reset_vector[0] = 0;
    warm_reset_vector[1] = boot_address >> 4;

    LocalAPIC_WriteTo(0x0310/4, local_apic_id << 24);
    LocalAPIC_WriteTo(0x0300/4, 0x500 | 0x8000 | 0x4000);
    PIT_Delay(2);
    LocalAPIC_WriteTo(0x0300/4, 0x500 | 0x8000);
    PIT_Delay(2);

    for (size_t i = 0; i < 2; i++) {
        LocalAPIC_WriteTo(0x0310/4, local_apic_id << 24);
        LocalAPIC_WriteTo(0x0300/4, 0x600 | boot_address >> 12);
        PIT_Delay(2);
    }

	return true;
}

/*

https://chromium.googlesource.com/chromiumos/third_party/coreboot/+/firmware-link-2695.B/src/cpu/x86/lapic/lapic_cpu_init.c

*/
