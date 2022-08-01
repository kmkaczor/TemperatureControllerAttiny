#include "hardwaredefs.h"
#include "sevensegment.h"
#include "math.h"

/*
 *  The values of a seven segment displayu are encoded as follows:
 *
 *       _a_
 *      |   |
 *     f|   |b
 *      |_g_|
 *      |   |
 *     e|   |c
 *      |___|
 *        d   .dp
 *
 * Note that the .dp is the decimal point.
 * Bit order is: abcdefg dp
 * a is the most significant bit and decimal point is the least, thus 0b10000010 is a segment with only a and g enabled,
 * but decimal point disabled.
 *
 */

const digit_t DIGIT_TABLE[] =
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
        0b11101110, // A
        0b00111110, // B as b
        0b10011100, // C
        0b01111010, // D as d
        0b10011110, // E
        0b10001110, // F
        0b11000110, // G, best effort
        0b01101110, // H
        0b00100000, // I as i
        0b01111000, // J
        0b01101110, // K best effort
        0b00011100, // L
        0b00101011, // M best effort
        0b00101010, // N as n
        0b11111100, // O
        0b11001110, // P
        0b00111011, // Q best effort
        0b00001010, // R as r
        0b10110111, // S best effort
        0b01100010, // T as t
        0b01111100, // U
        0b01111101, // V best effort
        0b00111000, // W best effort
        0b01101111, // X best effort
        0b01110110, // Y
        0b10011110, // Z best effort
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

static digit_t invert_display_char(digit_t c)
{
    // Swap 0xE0 bits with 0x1C bits, then add non-swapped bits.
    return (((c & 0xE0) >> 3) | ((c & 0x1C) << 3) | (c & 0x3));
}

/**
 * @brief Initialize seven segment display struct
 *
 * @param td Display struct pointer.
 * @param num_digits Number of digits in display.
 * @param port Microcontroller port. All pins must use this port.
 * @param pinmap (Pointer to) Array of pins controlling digits on/off state, in order.
 * @param digits (Pointer to) Array containing output value of each digit/segment. See DIGIT_TABLE[] above.
 * @param options Options:
 *      0x1: Invert display (upside down)
 */
void init_sevseg(struct sevseg_display_t *td, const uint8_t num_digits,
                 volatile uint8_t *port, uint8_t *pinmap, const uint8_t opts, digit_t *digits)
{
    // Must have at least two digits for temperature. Single digit is not useful.
    if (num_digits < 2)
        return;

    td->num_digits = num_digits;
    td->step = (num_digits % 2 == 0) ? 3 : 2; // Mix refresh order to reduce flickering.
    td->port = port;
    td->pin_map = pinmap;
    td->options = opts;
    td->display_buffer = digits;

    for (uint8_t i = 0; i < num_digits; i++)
    {
        *_DDR(td->port) |= (1 << pinmap[i]);
        *td->port |= (1 << td->pin_map[i]);
        td->display_buffer[i] = DIGIT_TABLE[SEVSEG_NULL];
    }
}

/**
 * @brief Set the digit at index to seven segment represenatation
 *
 * @param td Seven
 * @param index
 * @param c
 * @param decimal Boolean, decimal point enabled if != 0
 * @return uint8_t
 */
digit_t set_digit(struct sevseg_display_t *td, uint8_t index, const char c, const uint8_t decimal)
{
    digit_t buff = ' ';

    if (c >= '0' && c <= '9')
        buff = DIGIT_TABLE[c - '0'];
    else if (c >= 'A' && c <= 'Z')
        buff = DIGIT_TABLE[c - 'A' + 10];
    else if (c == '-')
        buff = DIGIT_TABLE[SEVSEG_MINUS];
    else if (c == ' ')
        buff = DIGIT_TABLE[SEVSEG_NULL];

    if (td->options & SEVSEG_OPT_INVERT)
    {
        index = td->num_digits - index - 1;
        buff = invert_display_char(buff);
    }

    td->display_buffer[index] = buff | !!decimal;
    return td->display_buffer[index];
}

void setLCD_shiftreg(struct sevseg_display_t *td, struct shiftreg8_t *sr)
{
    // Remember last updated digit. Every ISR call updates this value.
    static uint8_t digit_to_update = {0};

    // Unset the priorly set digit (does nothing on first instance if run via ISR).
    *td->port &= ~(1 << td->pin_map[digit_to_update % td->num_digits]);
    digit_to_update = (digit_to_update + td->step) % td->num_digits;

    shiftOut8(sr, td->display_buffer[digit_to_update]);

    *td->port |= (1 << td->pin_map[digit_to_update % td->num_digits]);
}

/**
 * @brief This is buggy and causes compiled file size to be bloated on microcontrollers like the
 *        attiny44. Only left in for possible future reference.
 *
 * @param td
 * @param f
 */
/*
void set_display_float(struct sevseg_display_t *td, float f)
{
    uint8_t leadingzero = 0;
    uint16_t tint = (int)f * 10;

    if (f < 0)
    {
        tint *= -1;
        set_digit(td, 0, '-', 0);
    }
    else
        set_digit(td, 0, ' ', 0);
    if ((tint % 10) >= 5)
    {
        tint /= 10;
        tint++;
    }
    else
        tint /= 10;

    for (int i = 1; i < td->num_digits; i++)
    {
        uint16_t digit = ((tint / (int)pow(10, td->num_digits - 1 - i)) % 10);
        uint8_t cr = SEVSEG_ERROR_INDEX;
        if (digit == 0 && leadingzero == 0 && i != td->num_digits - 1)
            cr = ' ';
        else
        {
            leadingzero = 1;
            cr = digit + '0';
        }

        set_digit(td, i, cr, 0);
    }
}
*/

/**
 * @brief Set the display to an integer value.
 *
 * @param td
 * @param n
 */

void set_display_int(struct sevseg_display_t *td, int n)
{
    uint8_t r = 0;
    uint8_t is_neg = 0;

    if (n < 0)
    {
        n *= -1;
        set_digit(td, 0, '-', 0);
        is_neg = 1;
    }

    // As leftmost digit is element 0, we start from the last digit and work our way to zero.
    // If negative, we reserve 0 for the negative sign(.
    // Exit loop once the number is fully displayed to avoid leading zeroes.
    int8_t i = td->num_digits - 1;
    for (; i >= is_neg && n > 0; i--)
    {
        r = n % 10;
        n /= 10;

        set_digit(td, i, r + '0', 0);
    }

    if (i == td->num_digits - 1)
    {
        set_digit(td, i, '0', 0);
        i--;
    }
    // Clear unused digits (e.g. remove 3rd digit if temperature goes from 101 to 99).
    for (; i >= is_neg; i--)
    {
        set_digit(td, i, ' ', 0);
    }

    // Note that if the display does not have enough digits than the most significant digits may not show up.
}

/**
 * @brief Set the display to the specified word.
 *
 * @param td Display
 * @param word String
 * @todo Most chars are currently unimplemented.
 */
void set_display(struct sevseg_display_t *td, char *word, const uint8_t len)
{
    for (uint8_t i = 0; i < td->num_digits; i++)
    {
        if (word[i] == '\0' || i > len - 1)
            word[i] = ' ';
        set_digit(td, i, word[i], 0);
    }
}

void set_decimal(struct sevseg_display_t *td, const uint8_t n)
{
    td->display_buffer[n] |= SEVSEG_DECIMAL;
}

void unset_decimal(struct sevseg_display_t *td, const uint8_t n)
{
    td->display_buffer[n] &= 0;
}

// Untested
void invert_display(struct sevseg_display_t *td)
{
    for (uint8_t i = 0; i < td->num_digits; i++)
    {
        td->display_buffer[i] = (((td->display_buffer[i] & 0xE0) >> 3) | ((td->display_buffer[i] & 0x1C) << 3) | (td->display_buffer[i] & 0x3));

        /*
        buffer = td->display_buffer[i];

        // Switch a & d
        td->display_buffer[td->num_digits - i] = (buffer & 0x80) >> 3 | (buffer & 0x10) << 3;
        // Switch b & f
        td->display_buffer[td->num_digits - i] = (buffer & 0x40) >> 4 | (buffer & 0x4) << 4;
        // Switch c & e
        td->display_buffer[td->num_digits - i] = (buffer & 0x20) >> 2 | (buffer & 0x8) << 2;
        */
    }
}