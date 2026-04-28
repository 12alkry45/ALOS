#ifndef TIMER_H
#define TIMER_H

#include <lib/string.h>
#include <stdint.h>

#define PORT_TIMER_CTRL 0x43
#define PORT_TIMER_DATA 0x40

void init_timer(uint32_t frequency);

#endif
