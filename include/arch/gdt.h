#ifndef GDT_H
#define GDT_H

#include <stdint.h>

extern uint8_t gdt_start;
extern uint8_t gdt_end;
extern uint16_t gdt_descriptor;

#define CODE_SEG 0x08
#define DATA_SEG 0x10

void init_gdt();

#endif
