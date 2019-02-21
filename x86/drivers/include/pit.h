#ifndef X86_DRIVERS_PIT_H
#define X86_DRIVERS_PIT_H

#include <stdint.h>

#define PIT_IOPORT_CHAN0                 0x40
#define PIT_IOPORT_CHAN1                 0x41
#define PIT_IOPORT_CHAN2                 0x42
#define PIT_IOPORT_COMD                  0x43

#define PIT_CHANNEL_0                    (0x0 << 6)
#define PIT_CHANNEL_1                    (0x1 << 6)
#define PIT_CHANNEL_2                    (0x2 << 6)

#define PIT_ACCESS_LATCH                 (0x0 << 4)
#define PIT_ACCESS_LOBYTE                (0x1 << 4)
#define PIT_ACCESS_HIBYTE                (0x2 << 4)
#define PIT_ACCESS_LOHIBYTE              (0x3 << 4)

#define PIT_OPERMODE_INT_TERM_COUNT      (0x0 << 1)
#define PIT_OPERMODE_HARD_RETRIG_ONESHOT (0x1 << 1)
#define PIT_OPERMODE_RATE_GENERATOR      (0x2 << 1)
#define PIT_OPERMODE_SQWAVE_GENERATOR    (0x3 << 1)
#define PIT_OPERMODE_SOFT_TRIG_STROBE    (0x4 << 1)
#define PIT_OPERMODE_HARD_TRIG_STROBE    (0x5 << 1)

#define PIT_COUNTMODE_BIN                 0x0
#define PIT_COUNTMODE_BCD                 0x1

#define PIT_BASE_FREQUENCY                 1193182
#define PIT_TARGET_FREQUENCY               50

void     PIT_Initialize();
void     PIT_Reset();
uint16_t PIT_Read_Clock_Counter();

#endif
