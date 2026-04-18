#ifndef TIMER_H
#define TIMER_H

#include "../cpu/types.h"
#include "../lib/string.h"

#define PORT_TIMER_CTRL 0x43
#define PORT_TIMER_DATA 0x40

void init_timer(uint32_t frequency);

#endif
