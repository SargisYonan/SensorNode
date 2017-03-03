#include "can_transmit.h"

// TODO: write this

uint8_t can_transmit_count = 0;
uint8_t can_transmit_type_num = -1; // needs to be set on first creation of Can_Transmit

// sets an index of the can_transmit module array to be the new can_transmit's info
// also sets the fields accordingly
// RETURNS:
// the can_transmit with fields sest appropriately
// or a default module if too many can_transmits already exist
Can_Transmit new_can_transmit(uint8_t cur_type_num, volatile uint8_t *port,
    volatile uint8_t *pin, volatile uint8_t *ddr, uint8_t reg_bit) {
  Can_Transmit a = new_module();
  if (can_transmit_count >= ACTUATOR_MAX) {
    return a; // remember the key is that it has defaults set
  }
  if (can_transmit_count == 0) {
    can_transmit_type_num = cur_type_num;
  }
  a.type_num = can_transmit_type_num;
  a.type_string = CAN_TRANSMIT_IDENTIFIER_STRING;
  a.index = can_transmit_count++;
  a.port = port;
  a.pin = pin;
  a.ddr = ddr;
  a.reg_bit = reg_bit;
  a.init = &can_transmit_init;
  a.write = &can_transmit_write;
  a.destroy = &can_transmit_destroy;
  return a;
}

// currently a hardcoded solution
void *can_transmit_init(Can_Transmit a) {
  *a.ddr |= _BV(a.reg_bit); // pin 22
  return (void *) "Set to be an output for an can_transmit\r\n";
}

void *can_transmit_write(Can_Transmit a, void *origstr) {
  char *str = (char *) origstr;
  if (!(*a.ddr & _BV(a.reg_bit))) {
    return "Cannot write to can_transmit: DDR set to input\r\n";
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

void *can_transmit_destroy(Can_Transmit a) {
  *a.port &= ~_BV(a.reg_bit); // force port off before switching this off
  *a.ddr &= ~_BV(a.reg_bit);
  return (void *) "Cleared of any settings\r\n";
}
