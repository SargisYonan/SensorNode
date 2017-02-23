#include "actuator.h"

uint8_t actuator_count = 0;
uint8_t actuator_type_num = -1; // needs to be set on first creation of Actuator

// sets an index of the actuator module array to be the new actuator's info
// also sets the fields accordingly
// RETURNS:
// the actuator with fields sest appropriately
// or a default module if too many actuators already exist
Actuator new_actuator(uint8_t cur_type_num, volatile uint8_t *port,
    volatile uint8_t *pin, volatile uint8_t *ddr, uint8_t reg_bit) {
  Actuator a = new_module();
  if (actuator_count >= ACTUATOR_MAX) {
    return a; // remember the key is that it has defaults set
  }
  if (actuator_count == 0) {
    actuator_type_num = cur_type_num;
  }
  a.type_num = actuator_type_num;
  a.index = actuator_count++;
  a.port = port;
  a.pin = pin;
  a.ddr = ddr;
  a.reg_bit = reg_bit;
  a.init = &actuator_init;
  a.write = &actuator_write;
  a.destroy = &actuator_destroy;
  return a;
}

// currently a hardcoded solution
void *actuator_init(Actuator a) {
  *a.ddr |= _BV(a.reg_bit); // pin 22
  return (void *) "Set PIN 22 (PORTA0) to be an output for an actuator\r\n";
}

void *actuator_write(Actuator a, void *origstr) {
  char *str = (char *) origstr;
  if (!(*a.ddr & _BV(a.reg_bit))) {
    return "Cannot write to actuator: DDR set to input\r\n";
  }
  if (str[0] == '0') {
    *a.port &= ~_BV(a.reg_bit);
    strncpy(str, "OFF\r\n", 6);
  } else if (str[0] == '1') {
    *a.port |= _BV(a.reg_bit);
    strncpy(str, "ON\r\n", 5);
  } else {
    strncpy(str, "INVALID\r\n", 10);
  }
  return origstr;
}

void *actuator_destroy(Actuator a) {
  *a.port &= ~_BV(a.reg_bit); // force port off before switching this off
  *a.ddr &= ~_BV(a.reg_bit);
  return (void *) "PIN 22 is now cleared of any settings\r\n";
}
