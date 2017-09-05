#include "can_bus.h"

#include <avr/pgmspace.h>
#include <util/delay.h>

#include "uart.h"

static uint8_t can_bus_count = 0;

// sets an index of the can_bus module array to be the new can_bus's info
// also sets the fields accordingly
// RETURNS:
// the can_bus with fields sest appropriately
// or a default module if too many can_buss already exist
Can_Bus new_can_bus(uint8_t type_num, Can_Bus cb) {
  if (can_bus_count >= CAN_BUS_MAX) {
    return cb; // remember the key is that it has defaults set
  }
  cb.type_num = type_num;
  cb.init = &can_bus_init;
  cb.write = &can_bus_write;
  cb.read = &can_bus_read;
  cb.destroy = &can_bus_destroy;
  return cb;
}

// currently a hardcoded solution
void can_bus_init(Can_Bus cb) {
  if (cb.pin_count != 2) {
    uart_puts_P(
        PSTR("Can Bus needs to be initialized with 2 pins (TX, RX)\r\n"));
    return;
  }
  *cb.ddr[0] |= _BV(cb.reg_bit[0]); // TX is an output so set bit
  *cb.ddr[1] &= ~_BV(cb.reg_bit[1]); // RX is input so clear bit
  uart_puts_P(PSTR("Can Bus successfully initialized\r\n"));
}

void can_bus_read(Can_Bus cb, char *read_data, uint16_t max_bytes) {
  if (cb.pin_count != 2 ||
      ((*cb.ddr[0] & _BV(cb.reg_bit[0])) == 0) || // TX should be output
      ((*cb.ddr[1] & _BV(cb.reg_bit[1])) != 0)) { // RX should be input
    uart_puts_P(
        PSTR("Error: Can Bus not initialized with 2 pins\r\n"));
    return;
  }
  uart_printf("Can Bus RX is currently %s\r\n",
      *cb.pin[1] & _BV(cb.reg_bit[1]) ? "SET" : "CLEARED");
  snprintf(read_data, max_bytes, "CAN: %s\r\n",
      *cb.pin[1] & _BV(cb.reg_bit[1]) ? "SET" : "CLEARED");
}

void can_bus_write(Can_Bus cb, char *str, uint16_t max_bytes) {
  if (max_bytes) {} // stop complaining
  if (cb.pin_count != 2 ||
      ((*cb.ddr[0] & _BV(cb.reg_bit[0])) == 0) || // TX should be output
      ((*cb.ddr[1] & _BV(cb.reg_bit[1])) != 0)) { // RX should be input
    uart_puts_P(
        PSTR("Error: Can Bus not initialized with 2 pins\r\n"));
    return;
  }
  if (str[0] == 1) {
    *cb.port[0] |= _BV(cb.reg_bit[0]);
    _delay_us(50); // only allow it to turn on for a small amount of time (50us)
    *cb.port[0] &= ~_BV(cb.reg_bit[0]);
  }
  uart_puts_P(PSTR("Successfully actuated CAN BUS for 50 us\r\n"));
}

void can_bus_destroy(Can_Bus cb) {
  if (cb.pin_count != 2) {
    uart_puts_P(
        PSTR("Error: Can Bus not initialized with 2 pins\r\n"));
    return;
  }
  *cb.port[0] &= ~_BV(cb.reg_bit[0]); // clear pin before making it an input
  *cb.ddr[0] &= ~_BV(cb.reg_bit[0]); // pins should be inputs by default
  uart_puts_P(PSTR("Can Bus successfully de-initialized\r\n"));
}

