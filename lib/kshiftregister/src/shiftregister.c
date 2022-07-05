#include "shiftregister.h"

void init_shiftreg8(struct shiftreg8_t *sr, volatile uint8_t *port, uint8_t pin_latch,
                    uint8_t pin_clock, uint8_t pin_data)
{
    sr->port = port;
    sr->pin_latch = pin_latch;
    sr->pin_clock = pin_clock;
    sr->pin_data = pin_data;

    // Enable data direction enable to output.
    _DDR(*port) |= (1 << pin_data) | (1 << pin_clock) | (1 << pin_latch);
}

/**
 * @brief Apply value to shift register
 *
 * @param sr Shift register struct containing port and pins
 * @param sevseg_char
 */
void shiftOut8(struct shiftreg8_t *sr, uint8_t val)
{
    // Turn latch on, to high.
    *sr->port &= ~(1 << sr->pin_latch);

    // Set data pin to value
    for (uint8_t i = 0; i < 8; i++)
    {
        if (val & (1 << i))
            *sr->port |= 1 << sr->pin_data;
        else
            *sr->port &= ~(1 << sr->pin_data);

        // Pulse clock pin per bit
        *sr->port |= (1 << sr->pin_clock);
        *sr->port &= ~(1 << sr->pin_clock);
    }

    // Set latch off, to low.
    *sr->port |= (1 << sr->pin_latch);
}