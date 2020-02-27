#ifndef KERNEL_LOCALAPIC_H
#define KERNEL_LOCALAPIC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define LAPIC_REG_ID                  (0x0020/4)   // ID
#define LAPIC_REG_VERSION             (0x0030/4)   // Version
#define LAPIC_REG_TPR                 (0x0080/4)   // Task Priority
#define LAPIC_REG_APR                 (0x0090/4)   // Arbitration Priority
#define LAPIC_REG_PPR                 (0x00A0/4)   // Processor Priority
#define LAPIC_REG_EOI                 (0x00B0/4)   // EOI
#define LAPIC_REG_RRD                 (0x00C0/4)   // Remote Read 
#define LAPIC_REG_LDR                 (0x00D0/4)   // Logical Destination 
#define LAPIC_REG_DFR                 (0x00E0/4)   // Destination Format
#define LAPIC_REG_SIVR                (0x00F0/4)   // Spurious Interrupt Vector
#define LAPIC_REG_ESR                 (0x0280/4)   // Error Status
#define LAPIC_REG_CMCI                (0x02F0/4)   // Corrected Machine Check Interrupt
#define LAPIC_REG_ICRLO               (0x0300/4)   // Interrupt Command
#define LAPIC_REG_ICRHI               (0x0310/4)   // Interrupt Command [63:32]
#define LAPIC_REG_TIMER               (0x0320/4)   // Local Vector Table : Timer
#define LAPIC_REG_TSR                 (0x0330/4)   // Local Vector Table : Thermal Sensor
#define LAPIC_REG_PCMR                (0x0340/4)   // Local Vector Table : Performance Monitoring Counters
#define LAPIC_REG_LINT0               (0x0350/4)   // Local Vector Table : LINT0
#define LAPIC_REG_LINT1               (0x0360/4)   // Local Vector Table : LINT1
#define LAPIC_REG_ERROR               (0x0370/4)   // Local Vector Table : Error
#define LAPIC_REG_TICR                (0x0380/4)   // Timer Initial Count
#define LAPIC_REG_TCCR                (0x0390/4)   // Timer Current Count
#define LAPIC_REG_TDCR                (0x03E0/4)   // Timer Divide Configuration

#define LAPIC_LVT_INTR_VECTOR         0xFF
#define LAPIC_STARTUP_VECTOR(x)       (x >> 12)

#define LAPIC_LVT_DELIVERY_MODE       0x700
#define LAPIC_LVT_DELIVERY_FIXED      0
#define LAPIC_LVT_DELIVERY_SMI        (2 << 8)
#define LAPIC_LVT_DELIVERY_NMI        (4 << 8)
#define LAPIC_LVT_DELIVERY_INIT       (5 << 8)
#define LAPIC_LVT_DELIVERY_EXTINT     (7 << 8)
#define LAPIC_LVT_DELIVERY_STATUS     0x1000
#define LAPIC_LVT_DELIVERY_IDLE       0
#define LAPIC_LVT_DELIVERY_PENDING    (1 << 12)

#define LAPIC_LVT_PIN_POLARITY        0x2000
#define LAPIC_LVT_REMOTE_IRR          0x4000
#define LAPIC_LVT_TRIGGER_MODE        0x8000
#define LAPIC_LVT_MASKED              0x10000

#define LAPIC_LVT_TIMER_MODE          0x60000
#define LAPIC_LVT_TIMER_MODE_1SHOT    0
#define LAPIC_LVT_TIMER_MODE_PERIODIC (1 << 17)
#define LAPIC_LVT_TIMER_MODE_TSC      (2 << 17)

#define LAPIC_TDCR_DIVIDE_BY_2        0
#define LAPIC_TDCR_DIVIDE_BY_4        1
#define LAPIC_TDCR_DIVIDE_BY_8        2
#define LAPIC_TDCR_DIVIDE_BY_16       3
#define LAPIC_TDCR_DIVIDE_BY_32       4
#define LAPIC_TDCR_DIVIDE_BY_64       5
#define LAPIC_TDCR_DIVIDE_BY_128      6
#define LAPIC_TDCR_DIVIDE_BY_1        7

#define LAPIC_ICR_INTR_VECTOR         0xFF

#define LAPIC_ICR_DELIVERY_MODE       0x700
#define LAPIC_ICR_DELIVERY_FIXED      0
#define LAPIC_ICR_DELIVERY_LOWPRIO    (1 << 8)
#define LAPIC_ICR_DELIVERY_SMI        (2 << 8)
#define LAPIC_ICR_DELIVERY_NMI        (4 << 8)
#define LAPIC_ICR_DELIVERY_INIT       (5 << 8)
#define LAPIC_ICR_DELIVERY_STARTUP    (6 << 8)
#define LAPIC_ICR_DELIVERY_STATUS     0x1000
#define LAPIC_ICR_DELIVERY_IDLE       0
#define LAPIC_ICR_DELIVERY_PENDING    (1 << 12)

#define LAPIC_ICR_DEST_MODE           0x800

#define LAPIC_ICR_LEVEL               0x4000
#define LAPIC_ICR_LEVEL_DEASSERT      0
#define LAPIC_ICR_LEVEL_ASSERT        (1 << 14)

#define LAPIC_ICR_TRIGGER_MODE        0x8000
#define LAPIC_ICR_TRIGGER_EDGE        0
#define LAPIC_ICR_TRIGGER_LEVEL       (1 << 15)

#define LAPIC_ICR_DEST_SHORT          0x60000
#define LAPIC_ICR_DEST_NOSHORT        0
#define LAPIC_ICR_DEST_SELF           (1 << 18)
#define LAPIC_ICR_DEST_ALLSELF        (2 << 18)
#define LAPIC_ICR_DEST_ALLNOSELF      (3 << 18)

#define LAPIC_SIVR_INTR_VECTOR        0xFF
#define LAPIC_SIVR_INTR_SPURIOUS      0xFF
#define LAPIC_SIVR_SOFT_ENABLE        0x100

struct IContext;

extern uintptr_t LocalAPIC_address;
extern size_t    LocalAPIC_Num;
extern uintptr_t LocalAPIC_InfoPtrs[];

extern bool      LocalAPIC_Initialize();
extern uint32_t  LocalAPIC_WriteTo(size_t index, uint32_t value);
extern uint32_t  LocalAPIC_ReadFrom(size_t index);
extern void      LocalAPIC_EOI();
extern uint8_t   LocalAPIC_ID();

extern size_t    LocalAPIC_GetTimerFrequency(size_t iterations);
extern void      LocalAPIC_Timer_HandleInterrupt(struct IContext* frame);
extern void      LocalAPIC_Initialize_Timer(uint8_t vector, size_t freq);

#endif
