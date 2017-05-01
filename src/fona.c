
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
  f.read = &fona_read;
  f.write = &fona_write;
  f.destroy = &fona_destroy;
  fona_count++;
  return f;
}

// note that the fona will operate on a UART
// this function will set it to transparent TCP server mode
void fona_init(Fona f) {
  if (f.pin_count != 2) { // TODO: how many pins will the fona need?
    uart_puts_P(PSTR(
          "Fona not initialized due to having more or less than 2 pins\r\n"));
    return;
  }

  //TODO: get the FONA and connect up UART1
  unsigned char strbuf[RX_BUF_SIZE]; // hold the returned strings
  uint8_t strbuf_ptr = 0; // pointer to end of strbuf

  // TODO: the following will eventually need to provide baudrate, etc as param
  uart1_init();

  uart1_puts_P(PSTR("AT+CIPMODE=1\r\n")); // transparent mode on
  uart1_flushTX();

  // TODO: the following needs to go into a macro or function or something
  for (;;) {
    uint16_t bytes_read = uart1_ngetc(strbuf, strbuf_ptr, 5, RX_BUF_SIZE);
    if (bytes_read == (uint16_t) -1) {
      // tell the user this didn't work
      uart_puts_P(PSTR("Error with transparent mode"));
      return;
    }
    strbuf_ptr += bytes_read;
    if (strbuf[strbuf_ptr - 1] == '\r') break; // got the string
  }
  // TODO: check that the string contains "OK"
  // TODO: end of macro stuff

  uart1_puts_P(PSTR("AT+CSTT=\"CMNET\"\r\n")); // start task and TODO: set APN
  // TODO: check for "OK"
  uart1_puts_P(PSTR("AT+CIPSERVER=1,1234\r\n")); // start server on port 123
  // TODO: check for "OK"
  // TODO: check for "SERVER OK"


  uart_puts_P(PSTR("Fona initialized\r\n"));
  // TODO: send local ip address (which is also server IP) with AT+CIFSR
}

// TODO: read from appropriate UART. If setup correctly, will redirect from TCP
void fona_read(Fona f) {
  uart_puts_P(PSTR("fona_read() not yet ready\r\n"));
}

// TODO: write to appropriate uart. if setup correctly, will forward to TCP
void fona_write(Fona f, char *str) {
  uart_puts_P(PSTR("fona_write() not yet ready\r\n"));
}

// TODO: clear settings and ideally put to sleep
void fona_destroy(Fona f) {
  if (f.pin_count != 2) {
    uart_puts_P(PSTR(
          "Fona not destroyed due to having more or less than 2 pins\r\n"));
    return;
  }
  // TODO: destroy uart setup both on fona and on sensornode
  // TODO: send "AT+CIPSERVER=0\r\n"
  // TODO: send "AT+CIPCLOSE\r\n"
  uart_puts_P(PSTR("Cleared of any settings\r\n"));
}
