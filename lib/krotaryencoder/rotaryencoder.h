#include "hardwaredefs.h"

struct rotary_encoder_t
{
    uint8_t port;
    uint8_t pin_sw;
    uint8_t pin_dt;
    uint8_t pin_clk;

    uint8_t sw;
    uint8_t dt;
    uint8_t clk;
};

void init_rotary_encoder(struct rotary_encoder_t *enc, uint8_t port, uint8_t sw, uint8_t dt, uint8_t clk);
