#include "thermistor.h"
#include "math.h"
#include "hardwaredefs.h"
#include <avr/io.h>
#include <avr/common.h>

/**
 * @brief Obtain temperature readings for the thermistor.
 * @return float
 */

void init_thermistor(struct thermistor_t *t, volatile uint8_t *port, uint8_t pin, uint16_t bcoefficient,
                     uint16_t series_resistor, uint16_t resistance_nominal, int8_t temp_nominal)
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
    for (uint8_t i = 0; i < THERMISTOR_TEMPERATURE_SAMPLES; i++)
    {
        t->temperatures[i] = get_temperatureF(t);
        for (uint8_t t = 0; t < THERMISTOR_READING_CYCLES_DELAY; t++)
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
    float average;

    for (uint8_t i = 0; i < NOISE_REDUCTION_SMOOTHING_READINGS; i++)
    {
        // temps[i] = adc(t->pin);
        average += adc(t->pin); // temps[i];
        for (uint8_t t = 0; t < THERMISTOR_READING_CYCLES_DELAY; t++)
            ;
    }
    average /= NOISE_REDUCTION_SMOOTHING_READINGS;
    average = t->series_resistor / (ADC_MAX / average - 1);
    // b-parameter equation:
    // (1 / T) = (1 / To) + ln(R/Ro)
    // T = temperature (in kelvins)
    // To = Temperature nominal (in kelvins)
    // R = Measured resistance
    // Ro = Resistance nominal (of thermistor at temperature nominal)

    average = average / t->resistance_nominal;
    average = (log(average) * 1000) / t->bcoefficient / 1000 + (1 / (t->temperature_nominal + 273.15));
    return (1 / average - 273.15);
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
