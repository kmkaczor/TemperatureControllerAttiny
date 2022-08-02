#include "thermistor.h"
#include "math.h"
#include "hardwaredefs.h"
#include <avr/io.h>
#include <avr/common.h>

#define THERMISTOR_READ_ERROR_THRESHOLD 25
/**
 * @brief Return temperature reading from a thermistor in fahrenheit.
 * @todo Refactor if celsius needed
 *
 * @param t Thermistor object
 * @return float
 */
static float get_thermistor_temperature(struct thermistor_t *t)
{
    // A simplified version of the Steinhart-Hart equation.
    // int temps[NOISE_REDUCTION_SMOOTHING_READINGS];
    float average = 0;

    for (uint8_t i = 0; i < NOISE_REDUCTION_SMOOTHING_READINGS; i++)
    {
        // temps[i] = adc(t->pin);
        average += adc(t->pin); // temps[i];

        for (uint8_t t = 0; t < THERMISTOR_READING_CYCLES_DELAY; t++)
            ;
    }
    average /= NOISE_REDUCTION_SMOOTHING_READINGS;

    // If we get a reading within error threshold, set error status.
    // Prevent on/off functionality on bad readings or if thermistor goes bad.
    // Reason: for chicken coop usage, if thermistor dies and it's 100 degrees outside, don't want to cook
    // the chickens by turning the heat lamp on by default!
    // Might be more efficient with PTC thermistors. TODO.
    // NOTE: Tested only with NTC thermistor!
    if (average <= THERMISTOR_READ_ERROR_THRESHOLD || average >= ADC_MAX - THERMISTOR_READ_ERROR_THRESHOLD)
        t->thermistor_error = 1;

    average = t->series_resistor / (ADC_MAX / average - 1);
    // b-parameter equation:
    // (1 / T) = (1 / To) + ln(R/Ro)
    // T = temperature (in kelvins)
    // To = Temperature nominal (in kelvins)
    // R = Measured resistance
    // Ro = Resistance nominal (of thermistor at temperature nominal)

    average = average / t->resistance_nominal;
    average = log(average) / t->bcoefficient + (1 / (t->temperature_nominal + 273.15));
    // average = (log(average) * 1000) / t->bcoefficient / 1000 + (1 / ((t->temperature_nominal + 27315) / 100));
    average = (1 / average - 273.15);

#if TEMPERATURE_SCALE == FAHRENHEIT
    average = (average * 9.0) / 5.0 + 32.0;
#endif

    return average;
}
/**
 * @brief Obtain temperature readings for the thermistor.
 * @return float
 */

void init_thermistor(struct thermistor_t *t, volatile uint8_t *port, const uint8_t pin, const uint16_t bcoefficient,
                     const uint16_t series_resistor, const uint16_t resistance_nominal, const int8_t temp_nominal)
{
    t->port = port;
    t->pin = pin;
    t->bcoefficient = bcoefficient;
    t->series_resistor = series_resistor;
    t->resistance_nominal = resistance_nominal;
    t->temperature_nominal = temp_nominal;
    t->thermistor_error = 0;

    t->index = 0;
    // init_temperature(t);
    for (uint8_t i = 0; i < THERMISTOR_TEMPERATURE_SAMPLES; i++)
    {
        t->temperatures[i] = get_thermistor_temperature(t);
        for (uint8_t t = 0; t < THERMISTOR_READING_CYCLES_DELAY; t++)
            ;
    }
    _DDR(*port) &= ~(1 << pin);
}

/**
 * @brief Initialize thermistor struct's internal temperature log (for smoothing readings)
 *
 * @param t
 */
/*
static void init_temperature(struct thermistor_t *t)
{
    for (uint8_t i = 0; i < THERMISTOR_TEMPERATURE_SAMPLES; i++)
    {
        t->temperatures[i] = get_temperature(t);
        for (uint8_t t = 0; t < THERMISTOR_READING_CYCLES_DELAY; t++)
            ;
    }
}
*/

void log_temperature(struct thermistor_t *t)
{
    t->temperatures[t->index++ % THERMISTOR_TEMPERATURE_SAMPLES] = get_thermistor_temperature(t);
}

float get_temperature(const struct thermistor_t *t)
{
    float temp_avg = 0;

    for (uint16_t i = 0; i < THERMISTOR_TEMPERATURE_SAMPLES; i++)
        temp_avg += t->temperatures[i];

    return temp_avg / THERMISTOR_TEMPERATURE_SAMPLES;
}
