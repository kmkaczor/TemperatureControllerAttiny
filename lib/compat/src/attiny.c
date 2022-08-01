#include <avr/io.h>
#include <avr/eeprom.h>

// Turn this into interrupt later
uint16_t adc(uint8_t pin)
{
  ADCSRA |= (1 << ADSC); // Start ADC conversion. Turns off in hardware after completion.
  ADMUX = pin;

  // PORT_RELAY |= (1 << PB0);

  while (ADCSRA & (1 << ADSC))
    ;

  return ADC;
}