#ifndef _KOREY_ATTINY
#define _KOREY_ATTINY

#include <avr/common.h>
#include <avr/io.h>

#define _DDR(port) (*(&port - 1)) // Attiny DDRx registers are at one byte lower address
uint16_t adc(uint8_t pin);

#endif