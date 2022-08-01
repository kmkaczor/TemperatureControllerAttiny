#ifndef _THERMISTOR_KOREY
#define _THERMISTOR_KOREY

#include "hardwaredefs.h"

// Does arduino or attiny44 have this defined elsewhere?
#define ADC_MAX 1023

// Set temperature scale to be used.
#define FAHRENHEIT 0
#define CELSIUS 1
#define TEMPERATURE_SCALE FAHRENHEIT

// Reading average per individual temperature readings.
#define NOISE_REDUCTION_SMOOTHING_READINGS 5
// Amount of temperature samples to log in structure.
#define THERMISTOR_TEMPERATURE_SAMPLES 20
// Delay between temperature readings.
#define THERMISTOR_READING_CYCLES_DELAY 10 // in processor cycles

struct thermistor_t
{
    volatile uint8_t *port;
    uint8_t pin;
    int8_t temperature_nominal;
    uint8_t index;
    uint8_t thermistor_error;
    uint16_t bcoefficient;
    uint16_t series_resistor;
    uint16_t resistance_nominal;
    int16_t temperatures[THERMISTOR_TEMPERATURE_SAMPLES];
};

// Initialize thermistor
void init_thermistor(struct thermistor_t *t, volatile uint8_t *port, const uint8_t pin, const uint16_t bcoefficient,
                     const uint16_t series_resistor, const uint16_t resistance_nominal, const int8_t temp_nominal);

// float get_thermistor_temperature(struct thermistor_t *t);
float get_temperature(const struct thermistor_t *t);

void log_temperature(struct thermistor_t *t);

#endif