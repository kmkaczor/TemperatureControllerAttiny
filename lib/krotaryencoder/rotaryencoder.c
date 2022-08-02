#include "rotaryencoder.h"

#define MASK_SW 0x4
#define MASK_DT 0x2
#define MASK_CLK 0x1
#define MASK_ROT (MASK_CLK | MASK_DT)
#define MASK_ALL (MASK_SW | MASK_CLK | MASK_DT)

/**
 * @brief Initialize the rotary encoder, set data direction and pullup resistor enable.
 *
 * @param re Rotary Encoder struct
 * @param port Port used by pins
 * @param sw SW pin (button)
 * @param dt DT pin
 * @param clk CLK pin
 */
void init_rotary_encoder(struct rotary_encoder_t *re, volatile uint8_t *port, const uint8_t sw, const uint8_t dt, const uint8_t clk)
{
    re->port = port;
    re->pin_sw = sw;
    re->pin_dt = dt;
    re->pin_clk = clk;

    // Data direction: input
    _DDR(*port) &= ~(1 << sw);  // Input SW
    _DDR(*port) &= ~(1 << dt);  // Input DT
    _DDR(*port) &= ~(1 << clk); // Input CLK

    // Pullup resistor enable
    *port |= (1 << sw);
    *port |= (1 << dt);
    *port |= (1 << clk);
}

/**
 * @brief Returns bitmask of port status, in order of: SW, DT, CLK.
 *
 * @param re Rotary encoder struct
 * @return uint8_t  Returns bitmask of SW | DT | CLK.
 */
uint8_t get_rotenc_status(struct rotary_encoder_t *re)
{
    return !!(_PIN(*re->port) & (1 << re->pin_sw)) << 2 |
           !!((_PIN(*re->port) & (1 << re->pin_dt))) << 1 |
           !!((_PIN(*re->port) & (1 << re->pin_clk)));
}

uint8_t get_rotenc_sw(struct rotary_encoder_t *re)
{
    return !!(_PIN(*re->port) & (1 << re->pin_sw));
}
uint8_t get_rotenc_dt(struct rotary_encoder_t *re)
{
    return !!(_PIN(*re->port) & (1 << re->pin_dt));
}
uint8_t get_clk(struct rotary_encoder_t *re)
{
    return !!(_PIN(*re->port) & (1 << re->pin_clk));
}