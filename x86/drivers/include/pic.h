#ifndef X86_KERNEL_PIC_H
#define X86_KERNEL_PIC_H

#include <stdint.h>

/*

This is the interface to the 8259(A) PIC
We assume the standard PIC configuration :
- 8086 configuration (All x86 systems would be using this)
- One master PIC (IRQs 0 -  7) 
- One  slave PIC (IRQs 8 - 15)
- Slave connected to IRQ 2 of the master
- EOI to be acknowledged by the interrupt service provider

Some terminology :
IRQ --> Interrupt request. There are 8 IRQ lines on a PIC
ICW --> Initialization command word 
OCW --> Operation      command word

*/

#define PIC_IOPORT_COMD1       0x20                /* Command port    for master PIC */
#define PIC_IOPORT_DATA1       0x21                /* Data    port    for master PIC */
#define PIC_IOPORT_COMD2       0xA0                /* Command port    for  slave PIC */
#define PIC_IOPORT_DATA2       0xA1                /* Data    port    for  slave PIC */

#define PIC_EOI                0x20                /* End-of-interrupt command code */
 
#define PIC_ICW1_NEED_ICW4     0x01                /* ICW4 (not) needed           -- we need it and so the bit needs to be set */
#define PIC_ICW1_SINGLE        0x02                /* Single (cascade) mode       -- we will initialize in cascade mode i.e. one master and one slave PIC, and so this bit needs to be 0 */
#define PIC_ICW1_CALL4         0x04                /* Call address interval 4 (8) -- not relevant for 8086 mode that we will be using, set to 0 */
#define PIC_ICW1_LEVEL         0x08                /* Level triggered (edge) mode -- edge triggered */
#define PIC_ICW1_INIT          0x10                /* Initialization - required!  -- this bit needs to be set to perform the initialization sequence */
 
#define PIC_ICW4_8086          0x01                /* 8086/88 (MCS-80/85) mode    -- this is the mode we are interested in, should be set */
#define PIC_ICW4_AUTO_EOI      0x02                /* Auto (normal) EOI           -- normal EOI will be dispatched by the interrupt handler, set to 0 */
#define PIC_ICW4_BUF_SLAVE     0x08                /* Buffered mode/slave         -- no, set to 0 */
#define PIC_ICW4_BUF_MASTER    0x0C                /* Buffered mode/master        -- no, set to 0 */
#define PIC_ICW4_SFNM          0x10                /* Special fully nested (not)  -- no, set to 0 */

#define PIC_READ_IRR           0x0A                /* After sending this OCW3 command word to the PIC command port we can read the 8-bit IRR value from the command port */
#define PIC_READ_ISR           0x0B                /* After sending this OCW3 command word to the PIC command port we can read the 8-bit ISR value from the command port */ 

#define PIC_IRQ_OFFSET         0x20
#define PIC_NUM_IRQS           0x10
#define PIC_REMAP1_START       0+PIC_IRQ_OFFSET    /* Start interrupt vector for the remapped IRQs of the master PIC */
#define PIC_REMAP2_START       8+PIC_IRQ_OFFSET    /* Start interrupt vector for the remapped IRQs of the  slave PIC */

extern void      PIC_Initialize();

extern void      PIC_SendEOI(uint8_t irqline);

extern uint16_t  PIC_CombineInterruptMasks(uint8_t  mask1, uint8_t mask2);
extern void      PIC_EnableInterrupt      (uint8_t  irqline);
extern void      PIC_DisableInterrupt     (uint8_t  irqline);

extern void      PIC_DisableAllInterrupts        ();
extern void      PIC_SetInterruptMask            (uint16_t mask);
extern uint16_t  PIC_GetInterruptMaskRegister    (void);
extern uint16_t  PIC_GetInterruptRequestRegister (void);
extern uint16_t  PIC_GetInServiceRegister        (void);

extern void      PIC1_DisableAllInterrupts       ();
extern void      PIC1_SetInterruptMask           (uint8_t mask);
extern uint8_t   PIC1_GetInterruptMaskRegister   (void);
extern uint8_t   PIC1_GetInterruptRequestRegister(void);
extern uint8_t   PIC1_GetInServiceRegister       (void);

extern void      PIC2_DisableAllInterrupts       ();
extern void      PIC2_SetInterruptMask           (uint8_t mask);
extern uint8_t   PIC2_GetInterruptMaskRegister   (void);
extern uint8_t   PIC2_GetInterruptRequestRegister(void);
extern uint8_t   PIC2_GetInServiceRegister       (void);

#endif
