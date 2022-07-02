#ifndef _SEVEN_SEGMENT_KOREY
#define _SEVEN_SEGMENT_KOREY

#include <inttypes.h>
#include "shiftregister.h"

struct sevseg_display_t
{
    int num_digits; // Total amount of digits on display
    int step;       // Step for ISR calls (e.g. a step of 2 will refresh digits in 0, 2, 4, 6 (modulus num_digits) order)
    volatile uint8_t *port;
    uint8_t *pin_map;
    uint8_t *dispdigit;
};

void init_sevseg(struct sevseg_display_t *, int num_digits, volatile uint8_t *port, uint8_t *pinmap, uint8_t *digits);
void set_display_float(struct sevseg_display_t *td, float f);
uint8_t set_digit(struct sevseg_display_t *td, int index, char c);

void sr_send(uint8_t sevseg_char);
void setLCD_shiftreg(struct sevseg_display_t *td, struct shiftreg8_t *sr);

#endif
