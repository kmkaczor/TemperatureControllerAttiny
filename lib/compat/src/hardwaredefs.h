#ifndef _KOREY_HARDWARE_DEFS
#define _KOREY_HARDWARE_DEFS

// Include hardware-specific definitions for easier portability
#ifdef __AVR
#include "attiny.h"

#elif ARDUINO > 100
#define adc(pin) analogRead(pin)

#define _DDR(port) (*(&port - 1)) // Attiny DDRx registers are at one byte lower address

#endif

#endif