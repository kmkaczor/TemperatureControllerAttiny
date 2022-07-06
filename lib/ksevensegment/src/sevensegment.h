#ifndef _SEVEN_SEGMENT_KOREY
#define _SEVEN_SEGMENT_KOREY

#include "shiftregister.h"

#define SEVSEG_DECIMAL 0x1
#define SEVSEG_MINUS 12
#define SEVSEG_ERROR_INDEX 13
#define SEVSEG_NULL 14

struct sevseg_display_t
{
    uint8_t num_digits; // Total amount of digits on display
    uint8_t step;       // Step for ISR calls (e.g. a step of 2 will refresh digits in 0, 2, 4, 6 (modulus num_digits) order)
    volatile uint8_t *port;
    uint8_t *pin_map;
    uint8_t *dispdigit;
};

void init_sevseg(struct sevseg_display_t *, uint8_t num_digits, volatile uint8_t *port, uint8_t *pinmap, uint8_t *digits);

uint8_t set_digit(struct sevseg_display_t *td, uint8_t index, char c);
void set_display_int(struct sevseg_display_t *td, int n);
void set_display(struct sevseg_display_t *td, char *word);

void set_display_float(struct sevseg_display_t *td, float f);

void setLCD_shiftreg(struct sevseg_display_t *td, struct shiftreg8_t *sr);

#endif
