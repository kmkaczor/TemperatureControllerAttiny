#include "shiftregister.h"

/**
 * @brief Initialize shift register.
 *
 * @param sr Pointer to shift register struct object.
 * @param port Microcontroller port. Pins must have same port value.
 * @param pin_latch Latch pin.
 * @param pin_clock Clock pin.
 * @param pin_data  Data pin.
 */
void init_shiftreg8(struct shiftreg8_t *sr, volatile uint8_t *port, const uint8_t pin_latch,
                    const uint8_t pin_clock, const uint8_t pin_data)
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
 * @param val Byte mapping for shit register
 */
void shiftOut8(struct shiftreg8_t *sr, const uint8_t val)
{
    // Turn latch on, to high.
    *sr->port &= ~(1 << sr->pin_latch);

    // Set data pin to val
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