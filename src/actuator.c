#include "actuator.h"

#include <avr/pgmspace.h>

#include "uart.h"

static uint8_t actuator_count = 0;

// sets an index of the actuator module array to be the new actuator's info
// also sets the fields accordingly
// RETURNS:
// the actuator with fields sest appropriately
// or a default module if too many actuators already exist
Actuator new_actuator(uint8_t type_num, Actuator a) {
  if (actuator_count >= ACTUATOR_MAX) {
    return a; // remember the key is that it has defaults set
  }
  a.type_num = type_num;
  a.init = &actuator_init;
  a.write = &actuator_write;
  a.destroy = &actuator_destroy;
  actuator_count++;
  return a;
}

// currently a hardcoded solution
void actuator_init(Actuator a) {
  if (a.pin_count != 1) {
    uart_puts_P(PSTR(
          "Actuator not initialized due to having more or less than 1 pin\r\n"));
    return;
  }
  *a.ddr[0] |= _BV(a.reg_bit[0]);
  uart_puts_P(PSTR("Actuator initialized\r\n"));
}

void actuator_write(Actuator a, char *str) {
  if (!(*a.ddr[0] & _BV(a.reg_bit[0]))) {
    uart_puts_P(PSTR("Cannot write to actuator: DDR set to input\r\n"));
    return;
  }
  if (str[0] == '0') {
    *a.port[0] &= ~_BV(a.reg_bit[0]);
    strncpy(str, "OFF\r\n", 6);
  } else if (str[0] == '1') {
    *a.port[0] |= _BV(a.reg_bit[0]);
    strncpy(str, "ON\r\n", 5);
  } else {
    strncpy(str, "INVALID\r\n", 10);
  }
  uart_printf(str);
}

void actuator_destroy(Actuator a) {
  if (a.pin_count != 1) {
    uart_puts_P(PSTR(
          "Actuator not destroyed due to having more or less than 1 pin\r\n"));
    return;
  }
  *a.port[0] &= ~_BV(a.reg_bit[0]); // force port off before switching this off
  *a.ddr[0] &= ~_BV(a.reg_bit[0]);
  uart_puts_P(PSTR("Cleared of any settings\r\n"));
}
