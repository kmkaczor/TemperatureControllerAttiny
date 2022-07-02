#include "thermistor.h"
#include "math.h"
#include "hardwaredefs.h"
#include <avr/io.h>
#include <avr/common.h>

/**
 * @brief Obtain temperature readings for the thermistor.
 * // SPDX-FileCopyrightText: 2011 Limor Fried/ladyada for Adafruit Industries
 * Slightly edited by Korey Kaczor. Thanks to Limor Fried and Adafruit for modified Steinhart-Hart equation!
 * @return float
 */

void init_thermistor(struct thermistor_t *t, volatile uint8_t *port, int pin, int bcoefficient,
                     int series_resistor, int resistance_nominal, int temp_nominal)
{
    t->port = port;
    t->pin = pin;
    t->bcoefficient = bcoefficient;
    t->series_resistor = series_resistor;
    t->resistance_nominal = resistance_nominal;
    t->temperature_nominal = temp_nominal;

    _DDR(*port) &= ~(1 << pin);
}

/**
 * @brief Initialize thermistor struct's internal temperature log (for smoothing readings)
 *
 * @param t
 */
void init_temperatureF(struct thermistor_t *t)
{
    for (int i = 0; i < THERMISTOR_TEMPERATURE_SAMPLES; i++)
    {
        t->temperatures[i] = get_temperatureF(t);
        for (int t = 0; t < THERMISTOR_READING_CYCLES_DELAY; t++)
            ;
    }
}

/**
 * @brief Return temperature reading from a thermistor in fahrenheit.
 * @todo Refactor if celsius needed
 *
 * @param t Thermistor object
 * @return float
 */
float get_temperature(struct thermistor_t *t)
{
    // A simplified version of the Steinhart-Hart equation.
    // int temps[NOISE_REDUCTION_SMOOTHING_READINGS];
    float average = 0;
    float steinhart = 0;

    for (int i = 0; i < NOISE_REDUCTION_SMOOTHING_READINGS; i++)
    {
        // temps[i] = adc(t->pin);
        average += adc(t->pin); // temps[i];
        for (int t = 0; t < THERMISTOR_READING_CYCLES_DELAY; t++)
            ;
    }

    average /= NOISE_REDUCTION_SMOOTHING_READINGS;
    average = ADC_MAX / average - 1;
    average = t->series_resistor / average;

    steinhart = average / t->resistance_nominal; // (R/Ro)
    steinhart = logf(steinhart);                 // ln(R/Ro)
    steinhart /= t->bcoefficient;                // 1/B * ln(R/Ro)
                                                 
    steinhart += 1.0 / (t->temperature_nominal + 273.15); // + (1/To)
   // steinhart += 1.0 / (t->temperature_nominal); // + (1/To)
    steinhart = 1.0 / steinhart;                 // Invert
    steinhart -= 273.15;                                  // convert absolute temp to C

    return steinhart;
}

float get_temperatureC(struct thermistor_t *t)
{
    return get_temperature(t);
}

float get_temperatureF(struct thermistor_t *t)
{
    float tempC = get_temperature(t);
    return (tempC * 9.0) / 5.0 + 32.0;
}
