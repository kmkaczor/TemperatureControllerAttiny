#ifndef _ROTARY_ENCODER_KOREY
#define _ROTARY_ENCODER_KOREY

#include "hardwaredefs.h"

struct rotary_encoder_t
{
    volatile uint8_t *port;
    uint8_t pin_sw;
    uint8_t pin_dt;
    uint8_t pin_clk;

    volatile uint8_t status;
};

void init_rotary_encoder(struct rotary_encoder_t *re, volatile uint8_t *port, const uint8_t sw, const uint8_t dt, const uint8_t clk);

uint8_t get_rotenc_status(struct rotary_encoder_t *re);
uint8_t get_rotenc_sw(struct rotary_encoder_t *re);
uint8_t get_rotenc_dt(struct rotary_encoder_t *re);
uint8_t get_rotenc_clk(struct rotary_encoder_t *re);

#endif