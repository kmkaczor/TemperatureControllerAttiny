#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>

#include "rotaryencoder.h"
#include "shiftregister.h"
#include "thermistor.h"
#include "sevensegment.h"

#define RELAY_PORT PORTA
#define RELAY_PIN PA7
#define TEMPERATURE_SCALE FAHRENHEIT

#define EEPROM_LOW_ADDY (uint16_t *)10
#define EEPROM_HIGH_ADDY (uint16_t *)20
#define TEMP_MIN -50
#define TEMP_MAX 150
#define TEMP_LOW_DEFAULT 32
#define TEMP_HIGH_DEFAULT 65

// How often every time OCR0A is reached
#define TIMER_INTERVAL 0.004992
// Seconds desired divided by our timer interval
#define TIME_TEMP_READING 400    // 2 / TIMER_INTERVAL
#define TIME_ROTENC_TIMEOUT 1000 // 5 / TIMER_INTERVAL

#define ROT_ENC_SW PB1
#define ROT_ENC_DT PB0
#define ROT_ENC_CLK PB2

static struct shiftreg8_t sr;
static struct sevseg_display_t ss1;
static struct thermistor_t t1;
static struct rotary_encoder_t re1;

enum display_state
{
  DISPLAY_AMBIENT_STATE,
  DISPLAY_HIGH_TEMP,
  DISPLAY_LOW_TEMP
} volatile td_state = {DISPLAY_AMBIENT_STATE};

enum rot_enc_event
{
  NONE,
  ROT_EVENT_BUTTON,
  ROT_EVENT_CCW,
  ROT_EVENT_CW,
} volatile rot_enc_state = {NONE};

#define TEMP_LOW_MIN -50 // Fahrenheit
#define TEMP_HIGH_MAX 200

volatile static unsigned int overflow = 0;
static int16_t temp_high_thresh;
static int16_t temp_low_thresh;
volatile static uint8_t read_temp = 0;

/**
 * @brief Initialize ports and pins.
 *
 */
void init_pins()
{
  // Relay data direction input
  _DDR(RELAY_PORT) |= (1 << RELAY_PIN);

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

  TCNT0 = 0;
  TCCR0A = (1 << WGM01);
  TCCR0B = (0 << CS02) | (1 << CS01) | (1 << CS00);
  TIMSK0 |= (1 << OCIE0A);
  OCR0A = 78; // 0.004992 sec with prescalar of 64 at 1 Mhz.

  sei();
}

/**
 * @brief Timer ISR
 *
 */
ISR(TIM0_COMPA_vect)
{
  // Refresh display before doing anything else.
  setLCD_shiftreg(&ss1, &sr);

  static uint8_t rotenc_last_position = 0b11;
  static unsigned int rotenc_overflow = 0;
  static unsigned int temperature_overflow = 0;
  // Bitmask of rotary encoder status. Three bits, SW, DT, and CLK.
  uint8_t rotenc_current = get_rotenc_status(&re1);

  // Allow temperature reading in main loop
  if ((unsigned int)(overflow - temperature_overflow) >= TIME_TEMP_READING) // 2 Seconds
  {
    temperature_overflow = overflow;
    read_temp = 1;
  }

  if ((rotenc_current & 0b100) == 0b000 && (rotenc_last_position & 0b100) == 0b100) // Active low
  {
    rot_enc_state = ROT_EVENT_BUTTON;
    rotenc_overflow = overflow;
  }

  // 11 -> 10; 00 -> 01 CCW
  // 11 -> 01; 00 -> 10 CW
  if ((rotenc_last_position & 0b11) == 0b11)
  {
    if ((rotenc_current & 0b11) == 0b11)
    {
      // No change
    }
    else if ((rotenc_current & 0b11) == 0b10)
    {
      rot_enc_state = ROT_EVENT_CCW;
      // CCW
    }
    else if ((rotenc_current & 0b11) == 0b01)
    {
      rot_enc_state = ROT_EVENT_CW;
      // CW
    }
  }
  else
    rotenc_overflow = overflow;

  rotenc_last_position = rotenc_current;

  // Save temperature limits to EEPROM if changed on user input timeout.
  if (td_state != DISPLAY_AMBIENT_STATE && (unsigned int)(overflow - rotenc_overflow) >= TIME_ROTENC_TIMEOUT)
  {
    eeprom_update_word(EEPROM_LOW_ADDY, temp_low_thresh);
    eeprom_update_word(EEPROM_HIGH_ADDY, temp_high_thresh);
    td_state = DISPLAY_AMBIENT_STATE;
  }

  overflow++;
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
  init_thermistor(&t1, &PORTA, PA6, 3950, 10000, 10000, 25);

  // Shift register (for seven segment display)
  init_shiftreg8(&sr, &PORTA, PA3, PA4, PA5);

  init_rotary_encoder(&re1, &PORTB, PB1, PB0, PB2);

  // Seven Segment Display
  static uint8_t sevseg_pin_map[] =
      {PA0,
       PA1,
       PA2};

  uint8_t num_digits = sizeof(sevseg_pin_map) / sizeof(sevseg_pin_map[0]);
  uint8_t digits[num_digits];
  init_sevseg(&ss1, num_digits, &PORTA, sevseg_pin_map, SEVSEG_OPT_INVERT, digits);

  // Read EEPROM for temperature values.
  temp_low_thresh = eeprom_read_word(EEPROM_LOW_ADDY);
  if (temp_low_thresh == 0xFFFF)
    temp_low_thresh = TEMP_LOW_DEFAULT;
  temp_high_thresh = eeprom_read_word(EEPROM_HIGH_ADDY);
  if (temp_high_thresh == 0xFFFF)
    temp_high_thresh = TEMP_HIGH_DEFAULT;
}

int main()
{
  setup();

  while (1)
  {
    // Read_temp flag from ISR routine
    if (read_temp == 1)
    {
      read_temp = 0;
      log_temperature(&t1);
    }

    // Returned averaged value (more accurate that prior log reading)
    int16_t temperature = get_temperature(&t1);
    if (t1.thermistor_error != 0)
    {
      set_digit(&ss1, 0, 'E', 0);
      set_digit(&ss1, 1, 'R', 0);
      set_digit(&ss1, 2, 'R', 0);

      RELAY_PORT &= ~(1 << RELAY_PIN);
      while (1)
        ;
    }
    else if (temperature <= temp_low_thresh)
      RELAY_PORT |= (1 << RELAY_PIN);
    else if (temperature >= temp_high_thresh)
      RELAY_PORT &= ~(1 << RELAY_PIN);

    // Handle rotary encoder events
    int incr = 0;
    switch (rot_enc_state)
    {
    case ROT_EVENT_CCW:
      if (td_state == DISPLAY_AMBIENT_STATE)
        td_state = DISPLAY_LOW_TEMP;
      incr = -1;
      break;

    case ROT_EVENT_CW:
      if (td_state == DISPLAY_AMBIENT_STATE)
        td_state = DISPLAY_LOW_TEMP;
      incr = 1;
      break;

    case ROT_EVENT_BUTTON:
      if (td_state == DISPLAY_AMBIENT_STATE)
        td_state = DISPLAY_LOW_TEMP;
      else if (td_state == DISPLAY_LOW_TEMP)
        td_state = DISPLAY_HIGH_TEMP;
      else if (td_state == DISPLAY_HIGH_TEMP)
        td_state = DISPLAY_LOW_TEMP;
      break;

    default:
    case NONE:
      break;
    }
    rot_enc_state = NONE;

    // Handle display state
    switch (td_state)
    {
    case DISPLAY_HIGH_TEMP:
      if (incr && (temp_high_thresh + incr >= TEMP_LOW_MIN && temp_high_thresh + incr <= TEMP_HIGH_MAX) && temp_high_thresh + incr > temp_low_thresh)
        temp_high_thresh += incr;
      set_display_int(&ss1, temp_high_thresh);
      set_decimal(&ss1, 0);
      break;

    case DISPLAY_LOW_TEMP:
      if (incr && (temp_low_thresh + incr >= TEMP_LOW_MIN && temp_low_thresh + incr <= TEMP_HIGH_MAX) && temp_low_thresh + incr < temp_high_thresh)
        temp_low_thresh += incr;
      set_display_int(&ss1, temp_low_thresh);
      set_decimal(&ss1, 2);
      break;

    default:
    case DISPLAY_AMBIENT_STATE:

      set_display_int(&ss1, temperature);
      break;
    }
    incr = 0;
  }

  // Should not reach here.
  //  set_digit(&ss1, 0, SEVSEG_ERROR_INDEX, 0);
}