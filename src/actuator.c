#include "actuator.h"

uint8_t actuator_count = 0;
uint8_t actuator_type_num = -1; // needs to be set on first creation of Actuator

// sets an index of the actuator module array to be the new actuator's info
// also sets the fields accordingly
// RETURNS:
// the actuator with fields sest appropriately
// or a default module if too many actuators already exist
Actuator new_actuator(uint8_t type_num, Actuator a) {
  if (actuator_count >= ACTUATOR_MAX) {
    return a; // remember the key is that it has defaults set
  }
  if (actuator_count == 0) {
    actuator_type_num = type_num;
  }
  a.type_num = actuator_type_num;
  a.type_str = ACTUATOR_IDENTIFIER_STRING;
  a.index = actuator_count++;
  a.init = &actuator_init;
  a.write = &actuator_write;
  a.destroy = &actuator_destroy;
  return a;
}

// currently a hardcoded solution
void *actuator_init(Actuator a) {
  if (a.pin_count != 1)
    return "Actuator not initialized due to having more or less than 1 pin\r\n";
  *a.ddr[0] |= _BV(a.reg_bit[0]); // pin 22
  return "Actuator initialized\r\n";
}

void *actuator_write(Actuator a, void *origstr) {
  char *str = (char *) origstr;
  if (!(*a.ddr[0] & _BV(a.reg_bit[0]))) {
    return "Cannot write to actuator: DDR set to input\r\n";
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
  return origstr;
}

void *actuator_destroy(Actuator a) {
  if (a.pin_count != 1)
    return "Actuator not destroyed due to having more or less than 1 pin\r\n";
  *a.port[0] &= ~_BV(a.reg_bit[0]); // force port off before switching this off
  *a.ddr[0] &= ~_BV(a.reg_bit[0]);
  return (void *) "Cleared of any settings\r\n";
}
