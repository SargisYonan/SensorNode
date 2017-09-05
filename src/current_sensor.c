#include "current_sensor.h"

#include <avr/pgmspace.h>
#include <util/delay.h>

#include "uart.h"

static uint8_t current_sensor_count = 0;

// sets an index of the current_sensor module array to be the new current_sensor's info
// also sets the fields accordingly
// RETURNS:
// the current_sensor with fields sest appropriately
// or a default module if too many current_sensors already exist
Current_Sensor new_current_sensor(uint8_t type_num, Current_Sensor cs) {
  if (current_sensor_count >= CURRENT_SENSOR_MAX) {
    return cs; // remember the key is that it has defaults set
  }
  cs.type_num = type_num;
  cs.init = &current_sensor_init;
  cs.read = &current_sensor_read;
  cs.destroy = &current_sensor_destroy;
  return cs;
}

void current_sensor_init(Current_Sensor cs) {
  if (cs.pin_count != 1) {
    uart_puts_P(
        PSTR("Current Sensor needs to be initialized with 1 pin\r\n"));
    return;
  }
  if (*cs.port[0] != PORTF) {
    uart_puts_P(
        PSTR("Error: All available ADC's are only found on port f\r\n"));
    return;
  }
  ADCSRA |= _BV(ADEN); // enable ADC
  ADCSRA |= _BV(ADPS2) | _BV(ADPS0); // set ADC prescaler to CPU_F/128, ~125kHz

  uart_puts_P(PSTR("Current Sensor successfully initialized\r\n"));
}

void current_sensor_read(Current_Sensor cs, char *read_data, uint16_t max_bytes) {
  if (cs.pin_count != 1) {
    uart_puts_P(
        PSTR("Current Sensor needs to be initialized with 1 pin\r\n"));
    return;
  }
  if (*cs.port[0] != PORTF) {
    uart_puts_P(
        PSTR("Error: All available ADC's are only found on port f\r\n"));
    return;
  }
  ADMUX &= ~7; // clear lowest three bits to allow selection of an ADC channel
  ADMUX |= cs.reg_bit[0]; // as we want this will contain a value from 0 - 7

  ADCSRA |= _BV(ADSC); // start conversion of ADC
  while (!(ADCSRA & _BV(ADIF))); //wait until conversion done
  ADCSRA |= _BV(ADIF); // sometimes you gotta set it to clear it ...arduino...

  uint32_t adc = ADC;

  uart_printf("Current Sensor has adc reading of %lu\r\n",
      adc);
  snprintf(read_data, max_bytes, "%lu\r\n", adc);
}

void current_sensor_destroy(Current_Sensor cs) {
  if (cs.pin_count != 1) {
    uart_puts_P(
        PSTR("Error: Current Sensor not initialized with 1 pin\r\n"));
    return;
  }
  if (*cs.port[0] != PORTF) {
    uart_puts_P(
        PSTR("Error: All available ADC's are only found on port f\r\n"));
    return;
  }
  ADCSRA &= ~_BV(ADEN); // disable ADC
  uart_puts_P(PSTR("Current Sensor successfully de-initialized\r\n"));
}

