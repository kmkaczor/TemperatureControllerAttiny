#ifndef _THERMISTOR_KOREY
#define _THERMISTOR_KOREY

#include "hardwaredefs.h"

// Does arduino or attiny44 have this defined elsewhere?
#define ADC_MAX 1023

#define NOISE_REDUCTION_SMOOTHING_READINGS 5

// Thermistor and Steinhart-Hart defines
#define THERMISTOR_TEMPERATURE_SAMPLES 20

#define THERMISTOR_READING_SAMPLES 5
#define THERMISTOR_READING_CYCLES_DELAY 10 // in processor cycles

struct thermistor_t
{
    volatile uint8_t *port;
    int pin;
    int bcoefficient;
    int series_resistor;
    int resistance_nominal;
    int temperature_nominal;
    float temperatures[THERMISTOR_TEMPERATURE_SAMPLES];
};

void init_thermistor(struct thermistor_t *t, volatile uint8_t *port, int pin, int bcoefficient,
                     int series_resistor, int resistance_nominal, int temp_nominal);
void init_temperatureF(struct thermistor_t *t);

float get_temperature(struct thermistor_t *t);
float get_temperatureF(struct thermistor_t *t);

#endif