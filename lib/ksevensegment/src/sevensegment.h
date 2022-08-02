#ifndef _SEVEN_SEGMENT_KOREY
#define _SEVEN_SEGMENT_KOREY

#include "shiftregister.h"

#define SEVSEG_DECIMAL 0x1
#define SEVSEG_MINUS 36
#define SEVSEG_ERROR_INDEX 37
#define SEVSEG_NULL 38

#define SEVSEG_OPT_INVERT 0x1

typedef uint8_t digit_t;

struct sevseg_display_t
{
    uint8_t num_digits; // Total amount of digits on display
    uint8_t step;       // Step for ISR calls (e.g. a step of 2 will refresh digits in 0, 2, 4, 6 (modulus num_digits) order)
    volatile uint8_t *port;
    uint8_t *pin_map;
    digit_t *display_buffer;
    uint8_t options;
};

void init_sevseg(struct sevseg_display_t *, const uint8_t num_digits, volatile uint8_t *port, uint8_t * pinmap, const uint8_t opts, digit_t *digits);

digit_t set_digit(struct sevseg_display_t *td, uint8_t index, const char c, const uint8_t decimal);
void set_display_int(struct sevseg_display_t *td, int n);
void set_display(struct sevseg_display_t *td, char *word, const uint8_t len);

void setLCD_shiftreg(struct sevseg_display_t *td, struct shiftreg8_t *sr);

void set_decimal(struct sevseg_display_t *td, const uint8_t n);
void unset_decimal(struct sevseg_display_t *td, const uint8_t n);

void invert_display(struct sevseg_display_t *td);


#endif
