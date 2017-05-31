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
  cs.write = &current_sensor_write;
  cs.read = &current_sensor_read;
  cs.destroy = &current_sensor_destroy;
  return cs;
}

// currently a hardcoded solution
void current_sensor_init(Current_Sensor cs) {
  if (cs.pin_count != 2) {
    uart_puts_P(
        PSTR("Current Sensor needs to be initialized with 2 pins (TX, RX)\r\n"));
    return;
  }
  *cs.ddr[0] |= _BV(cs.reg_bit[0]); // TX is an output so set bit
  *cs.ddr[1] &= ~_BV(cs.reg_bit[1]); // RX is input so clear bit
  uart_puts_P(PSTR("Current Sensor successfully initialized\r\n"));
}

void current_sensor_read(Current_Sensor cs) {
  if (cs.pin_count != 2 ||
      ((*cs.ddr[0] & _BV(cs.reg_bit[0])) == 0) || // TX should be output
      ((*cs.ddr[1] & _BV(cs.reg_bit[1])) != 0)) { // RX should be input
    uart_puts_P(
        PSTR("Error: Current Sensor not initialized with 2 pins\r\n"));
    return;
  }
  uart_printf("Current Sensor RX is currently %s\r\n",
      *cs.pin[1] & _BV(cs.reg_bit[1]) ? "SET" : "CLEARED");
}

void current_sensor_write(Current_Sensor cs, char *str) {
  if (cs.pin_count != 2 ||
      ((*cs.ddr[0] & _BV(cs.reg_bit[0])) == 0) || // TX should be output
      ((*cs.ddr[1] & _BV(cs.reg_bit[1])) != 0)) { // RX should be input
    uart_puts_P(
        PSTR("Error: Current Sensor not initialized with 2 pins\r\n"));
    return;
  }
  if (str[0] == 1) {
    *cs.port[0] |= _BV(cs.reg_bit[0]);
    _delay_us(50); // only allow it to turn on for a small amount of time (50us)
    *cs.port[0] &= ~_BV(cs.reg_bit[0]);
  }
  uart_puts_P(PSTR("Successfully actuated CAN BUS for 50 us\r\n"));
}

void current_sensor_destroy(Current_Sensor cs) {
  if (cs.pin_count != 2) {
    uart_puts_P(
        PSTR("Error: Current Sensor not initialized with 2 pins\r\n"));
    return;
  }
  *cs.port[0] &= ~_BV(cs.reg_bit[0]); // clear pin before making it an input
  *cs.ddr[0] &= ~_BV(cs.reg_bit[0]); // pins should be inputs by default
  uart_puts_P(PSTR("Current Sensor successfully de-initialized\r\n"));
}

