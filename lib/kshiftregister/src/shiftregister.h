#ifndef _SHIFT_REGISTER_KOREY
#define _SHIFT_REGISTER_KOREY

#include "hardwaredefs.h"

struct shiftreg8_t
{
    volatile uint8_t *port; // Pins must be on same port
    uint8_t pin_latch;
    uint8_t pin_clock;
    uint8_t pin_data;
};

void init_shiftreg8(struct shiftreg8_t *sr, volatile uint8_t *port, const uint8_t pin_latch,
                    const uint8_t pin_clock, const uint8_t pin_data);
void shiftOut8(struct shiftreg8_t *sr, const uint8_t val);

#endif