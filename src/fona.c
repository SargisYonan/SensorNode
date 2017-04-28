
#include "fona.h"

#include <avr/pgmspace.h>

#include "uart.h"

static uint8_t fona_count = 0;
static uint8_t fona_type_num = -1; // needs to be set on first creation of Fona

// sets an index of the fona module array to be the new fona's info
// also sets the fields accordingly
// RETURNS:
// the fona with fields set appropriately
// or a default module if too many fonas already exist
Fona new_fona(uint8_t type_num, Fona f) {
  if (fona_count >= FONA_MAX) {
    return f; // remember the key is that it has defaults set
  }
  if (fona_count == 0) {
    fona_type_num = type_num;
  }
  f.type_num = fona_type_num;
  f.init = &fona_init;
  f.write = &fona_write;
  f.destroy = &fona_destroy;
  fona_count++;
  return f;
}

// currently a hardcoded solution
void fona_init(Fona f) {
  if (f.pin_count != 1) {
    uart_puts_P(PSTR(
          "Fona not initialized due to having more or less than 1 pin\r\n"));
    return;
  }
  *f.ddr[0] |= _BV(f.reg_bit[0]);
  uart_puts_P(PSTR("Fona initialized\r\n"));
}

void fona_write(Fona f, char *str) {
  if (!(*f.ddr[0] & _BV(f.reg_bit[0]))) {
    uart_puts_P(PSTR("Cannot write to fona: DDR set to input\r\n"));
    return;
  }
  if (str[0] == '0') {
    *f.port[0] &= ~_BV(f.reg_bit[0]);
    strncpy(str, "OFF\r\n", 6);
  } else if (str[0] == '1') {
    *f.port[0] |= _BV(f.reg_bit[0]);
    strncpy(str, "ON\r\n", 5);
  } else {
    strncpy(str, "INVALID\r\n", 10);
  }
  uart_printf(str);
}

void fona_destroy(Fona f) {
  if (f.pin_count != 1) {
    uart_puts_P(PSTR(
          "Fona not destroyed due to having more or less than 1 pin\r\n"));
    return;
  }
  *f.port[0] &= ~_BV(f.reg_bit[0]); // force port off before switching this off
  *f.ddr[0] &= ~_BV(f.reg_bit[0]);
  uart_puts_P(PSTR("Cleared of any settings\r\n"));
}
