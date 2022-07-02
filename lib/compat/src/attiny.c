#include <avr/io.h>

// Turn this into interrupt later
int adc(uint8_t pin)
{
  ADCSRA |= (1 << ADSC); // Start ADC conversion. Turns off in hardware after completion.
  ADMUX = pin;

  // PORT_RELAY |= (1 << PB0);

  while (ADCSRA & (1 << ADSC))
    ;

  return ADC;
}