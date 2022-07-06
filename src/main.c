#include <avr/interrupt.h>
#include <avr/io.h>
#include "shiftregister.h"
#include "thermistor.h"
#include "sevensegment.h"

#define RELAY_PORT PORTB
#define RELAY_PIN PB2
#define TEMP_F_ON_RELAY 80
#define TEMP_F_OFF_RELAY 82

#define ROT_ENC_SW PA0
#define ROT_ENC_DT PB2
#define ROT_ENC_CLK PB1

static struct shiftreg8_t sr;
static struct sevseg_display_t td;
static struct thermistor_t t1;

enum display_state
{
  DISPLAY_AMBIENT_STATE,
  DISPLAY_HIGH_TEMP,
  DISPLAY_LOW_TEMP
} volatile td_state;

volatile static unsigned long long overflow = 0;
/**
 * @brief Initialize ports and pins.
 *
 */
void init_pins()
{
  // Enable PCIEINT1 for rotary encoder.
  GIMSK |= (1 << PCIE1);

  // Relay to
  _DDR(RELAY_PORT) |= (1 << RELAY_PIN);

  // Initialize ADC operation
  // Enable ADC
  // Prescalar values: 2, 2, 4 8, 16, 32, 64, 128
  // ADC must be between 50 to 200 khz, prescale accordingly.
  ADCSRA |= (1 << ADEN) | (0 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 1 Mhz clock, use prescalar 8 to reach 125khz
}

/**
 * @brief Initialize timer register settings.
 *
 */
void init_timers()
{

  cli();
  TCCR0A = 0;
  TCCR1B = 0;
  TCNT0 = 0;
  TCCR0A |= (1 << WGM01);
  TCCR0B |= (0 << CS02) | (1 << CS01) | (1 << CS00);
  TIMSK0 |= (1 << OCIE0A);
  OCR0A = 78; // 0.004992 sec with prescalar of 64 at 1 Mhz.

  /*
 TCCR1A = 0x0; // set entire TCCR1A register to 0
 TCCR1B = 0x0; // same for TCCR1B
 TCNT1 = 0;    // initialize counter value to 0k

 // set compare match register for 1000 Hz increments
 // OCR1A = 15625; // 15625; // = 1000000 / (1 * 1000) - 1 (must be <65536)
 OCR1A = 625; // 15625; // = 1000000 / (1 * 1000) - 1 (must be <65536)

 //  turn on CTC mode
 TCCR1B |= (1 << WGM12);
 TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
 // enable timer compare interrupt
 TIMSK1 |= (1 << OCIE1A);
 */

  sei();
}

/**
 * @brief Timer ISR
 *
 */
ISR(TIM0_COMPA_vect)
{
  overflow++;
  setLCD_shiftreg(&td, &sr);
}

ISR(PCINT1_vect)
{
  /*
  static uint8_t last_pinout = 0x0;

  if (PORTB & (1 << ROT_ENC_CLK))
    ;
  if (PORTB & (1 << ROT_ENC_DT))
    ;
    */
}

/**
 * @brief Setup configuration prior to main loop.
 *
 */
void setup()
{
  init_pins();
  init_timers();

  // Thermistor setup
  init_thermistor(&t1, &PORTA, PA7, 3950, 10000, 10000, 25);
  init_temperatureF(&t1);

  // Shift register (for seven segment display)
  init_shiftreg8(&sr, &PORTA, PA3, PA4, PA5);

  // Seven Segment Display
  static uint8_t sevseg_pin_map[] =
      {PA1,
       PA0,
       PA2,
       PA6};

  uint8_t num_digits = sizeof(sevseg_pin_map) / sizeof(sevseg_pin_map[0]);
  uint8_t digits[num_digits];
  init_sevseg(&td, num_digits, &PORTA, sevseg_pin_map, digits);
}

int main()
{
  setup();
  uint8_t index = 0;

  while (1)
  {
    // Set temperature display to average of 20 readings to avoid fluctuations.
    int16_t t_avg = 0;
    t1.temperatures[index++ % THERMISTOR_TEMPERATURE_SAMPLES] = get_temperatureF(&t1);
    for (uint8_t i = 0; i < THERMISTOR_TEMPERATURE_SAMPLES; i++)
    {
      t_avg += t1.temperatures[i];
    }
    t_avg /= THERMISTOR_TEMPERATURE_SAMPLES;

    if (t_avg <= TEMP_F_ON_RELAY)
      RELAY_PORT |= (1 << RELAY_PIN);
    else if (t_avg >= TEMP_F_OFF_RELAY)
      RELAY_PORT &= ~(1 << RELAY_PIN);

    set_display_int(&td, t_avg);

    /*
        cli();
        _DDR(PORTA) &= ~(1 << PA0);
        //PORTA &= ~(1 << PA0);
        //PORTA |= (1 << PA0);

        if (PORTA & (1 << PA0))
          set_display_int(&td, "1234");

        _DDR(PORTA) |= (1 << PA0);
        sei();
        */
    // set_display_float(&td, analogRead(t1.pin));
  }

  set_digit(&td, 0, SEVSEG_ERROR_INDEX);
}