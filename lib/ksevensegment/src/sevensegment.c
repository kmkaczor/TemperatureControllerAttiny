#include "hardwaredefs.h"
#include "sevensegment.h"
#include "math.h"

/*
   _a_
  |   |
 f|   |b
  |_g_|
  |   |
 e|   |c
  |___|
    d   .dp
*/

// Bit order is:
// a b c d e f g dp
// a is MSB, . is LSB.
//  0babcdefgX
const uint8_t DIGIT_TABLE[] =
    {
        0b11111100, // 0
        0b01100000, // 1
        0b11011010, // 2
        0b11110010, // 3
        0b01100110, // 4
        0b10110110, // 5
        0b10111110, // 6
        0b11100000, // 7
        0b11111110, // 8
        0b11110110, // 9
        0b01101110, // H
        0b00011100, // L
        0b00000010, // -
        0b11111111, // Error
        0b00000000  // NULL

        // A
        // b
        // C
        // d
        // e
        // f
        // g
};
#define SEVSEG_DECIMAL 0x1
#define SEVSEG_MINUS 12
#define SEVSEG_ERROR_INDEX 13
#define SEVSEG_NULL 14

void init_sevseg(struct sevseg_display_t *td, int num_digits,
                 volatile uint8_t *port, uint8_t *pinmap, uint8_t *digits)
{
    // Must have at least two digits for temperature. Single digit is not useful.
    if (num_digits < 2)
        return;

    td->num_digits = num_digits;
    td->step = (num_digits % 2 == 0) ? 3 : 2;
    td->port = port;
    td->pin_map = pinmap;
    td->dispdigit = digits;

    for (int i = 0; i < num_digits; i++)
    {

        *_DDR(td->port) |= (1 << pinmap[i]);
        *td->port |= (1 << td->pin_map[i]);
        td->dispdigit[i] = DIGIT_TABLE[SEVSEG_NULL];
    }
}

/**
 * @brief Set the digit at index to seven segment represenatation
 *
 * @param td Seven
 * @param index
 * @param c
 * @return uint8_t
 */
uint8_t set_digit(struct sevseg_display_t *td, int index, char c)
{

    if (c >= '0' && c <= '9')
        td->dispdigit[index] = DIGIT_TABLE[c - '0'];
    else if (c == '-')
        td->dispdigit[index] = DIGIT_TABLE[SEVSEG_MINUS];
    else if (c == ' ')
        td->dispdigit[index] = DIGIT_TABLE[SEVSEG_NULL];

    return td->dispdigit[index];
}

void setLCD_shiftreg(struct sevseg_display_t *td, struct shiftreg8_t *sr)
{
    static int digit_to_update = {0};

    // Unset the priorly set digit (does nothing on first instance if run via ISR).
    *td->port &= ~(1 << td->pin_map[digit_to_update % td->num_digits]);
    digit_to_update = (digit_to_update + 1) % td->num_digits;

    //    sr_send(td->dispdigit[digit_to_update]);
    shiftOut8(sr, td->dispdigit[digit_to_update]);

    *td->port |= (1 << td->pin_map[digit_to_update % td->num_digits]);
}

void set_display_float(struct sevseg_display_t *td, float f)
{
    int leadingzero = 0;
    int tint = (int)f * 10;

    if (f < 0)
    {
        tint *= -1;
        set_digit(td, 0, '-');
    }
    else
        set_digit(td, 0, ' ');
    if ((tint % 10) >= 5)
    {
        tint /= 10;
        tint++;
    }
    else
        tint /= 10;

    for (int i = 1; i < td->num_digits; i++)
    {
        int digit = ((tint / (int)pow(10, td->num_digits - 1 - i)) % 10);
        int cr = SEVSEG_ERROR_INDEX;
        if (digit == 0 && leadingzero == 0 && i != td->num_digits - 1)
            cr = ' ';
        else
        {
            leadingzero = 1;
            cr = digit + '0';
        }

        set_digit(td, i, cr);
    }
}

void set_display(struct sevseg_display_t *td, char *word)
{
    for (int i = 0; i < td->num_digits; i++)
    {
        set_digit(td, i, word[i]);
    }
}
