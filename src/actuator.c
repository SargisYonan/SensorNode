#include "actuator.h"

uint8_t actuator_count = 0;
uint8_t actuator_type_num = -1; // needs to be set on first creation of Actuator

// sets an index of the actuator module array to be the new actuator's info
// also sets the fields accordingly
// RETURNS:
// the actuator with fields sest appropriately
// or a default module if too many actuators already exist
Actuator new_actuator(uint8_t cur_type_num) {
  Actuator a = new_module();
  if (actuator_count >= ACTUATOR_MAX) {
    return a; // remember the key is that it has defaults set
  }
  if (actuator_count == 0) {
    actuator_type_num = cur_type_num;
  }
  a.type_num = actuator_type_num;
  a.index = actuator_count++;
  a.init = &actuator_init;
  a.write = &actuator_write;
  return a;
}

// currently a hardcoded solution
void *actuator_init(void) {
  DDRA |= _BV(PA0); // pin 22
  return (void *) "Set PIN 22 (PORTA0) to be an output for an actuator\r\n";
}

void *actuator_write(void *origstr) {
  char *str = (char *) origstr;
  if (str[0] == '0') {
    PORTA &= ~_BV(PA0);
    strncpy(str, "OFF\r\n", 6);
  } else if (str[0] == '1') {
    PORTA |= _BV(PA0);
    strncpy(str, "ON\r\n", 5);
  } else {
    strncpy(str, "INVALID\r\n", 10);
  }
  return origstr;
}
