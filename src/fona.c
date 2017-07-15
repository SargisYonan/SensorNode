
#include "fona.h"

#include <avr/pgmspace.h>

#include "uart.h"

  static uint8_t fona_count = 0;

  // sets an index of the fona module array to be the new fona's info
  // also sets the fields accordingly
  // RETURNS:
  // the fona with fields set appropriately
  // or a default module if too many fonas already exist
  Fona new_fona(uint8_t type_num, Fona f) {
    if (fona_count >= FONA_MAX) {
      return f; // remember the key is that it has defaults set
    }
    f.type_num = type_num;
    f.init = &fona_init;
    f.read = &fona_read;
    f.write = &fona_write;
    f.destroy = &fona_destroy;
    //fona_count++;
    return f;
  }

  // note that the fona will operate on a UART
  // this function will set it to transparent TCP server mode
  void fona_init(Fona f) {
    if (f.pin_count != 3 || *f.port[0] != PORTD || *f.port[1] != PORTD ||
        f.reg_bit[0] != 2 || f.reg_bit[1] != 3) { // Rx1, TX1, RST
      uart_puts_P(PSTR(
            "Fona not initialized due to not being initialized on PD2 then PD3 then a third pin\r\n"));
      return;
    }
    uart1_init(4800);
/*
    //TODO: get the FONA and connect up UART1
    unsigned char strbuf[RX_BUF_SIZE]; // hold the returned strings
    uint8_t strbuf_ptr = 0; // pointer to end of strbuf


    uart1_puts_P(PSTR("AT+CIPMODE=1\r\n")); // transparent mode on
    uart1_flushTX();

    // Forget checking valid responses for now
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
    if (!strstr(strbuf, "AT+CIPMODE=1")) {
      uart_puts_P(PSTR("Error with initializing in transparent mode"));
      return;
    }
  // TODO: check that the string contains "OK"
  // TODO: end of macro stuff

  uart1_puts_P(PSTR("AT+CSTT=\"CMNET\"\r\n")); // start task and TODO: set APN
  // TODO: check for "OK"
  uart1_flushTX();
  uart1_puts_P(PSTR("AT+CIPSERVER=1,1234\r\n")); // start server on port 1234
  uart1_flushTX();
  // TODO: check for "OK"
  // TODO: check for "SERVER OK"
*/
  char strbuf[RX_BUF_SIZE];
  uart1_puts_P(PSTR("AT\r\n"));
  _delay_ms(100);
  uart1_ngetc(strbuf, 0, RX_BUF_SIZE, RX_BUF_SIZE); // clear newline
  _delay_ms(100);
  if(uart1_ngetc((unsigned char *) strbuf, 0, 20, RX_BUF_SIZE) == -1 || !strstr(strbuf, "OK")) {
    uart_printf("Error, invalid response from fona: %s\r\n", strbuf);
    return;
  }

  uart_puts_P(PSTR("Fona initialized\r\n"));
  // TODO: send local ip address (which is also server IP) with AT+CIFSR
}

// TODO: read from appropriate UART. If setup correctly, will redirect from TCP
void fona_read(Fona f) {
  // TODO basically call uart1_gets (TODO) twice, saving the second string but
  // also parsing out the leading "\r\n"
  uart_puts_P(PSTR("fona_read() not yet ready\r\n"));
}

// TODO: write to appropriate uart. if setup correctly, will forward to TCP
// Specifically, the abstractions for sending the data directly should already
// be handled at this point
void fona_write(Fona f, char *str) {
  // TODO: use uart1_puts() directly since transparent mode
  // -s means shell mode
  if (strcmp(str, "-s") != 0) {
    uart_puts_P(PSTR("fona_write() not yet ready\r\n"));
    return;
  }
  char strbuf[RX_BUF_SIZE];
  uint8_t strbuf_ptr = 0;
  uart_puts_P(PSTR("[SensorNode @ FONA ]$ "));
  for (;;) {
    uint8_t bytes_read = uart_ngetc((unsigned char *) strbuf, strbuf_ptr, RX_BUF_SIZE, RX_BUF_SIZE);
    if (bytes_read == -1) {
      strbuf_ptr = 0;
      uart_puts_P(PSTR("\r\nString cleared due to exceeding in size\r\n"));
      uart_puts_P(PSTR("[SensorNode @ FONA ]$ "));
      continue;
    }
    strbuf_ptr += bytes_read;
    if (strbuf_ptr > 0 && strbuf[strbuf_ptr - 1] == '\r') { // got the string
      if (strstr(strbuf, "-e")) break; // exit shell
      strbuf[strbuf_ptr - 1] = '\0';
      uart1_printf("%s\r\n", strbuf);
      _delay_ms(100);
      uart1_ngetc((unsigned char *) strbuf, 0, RX_BUF_SIZE, RX_BUF_SIZE); // clear newline
      _delay_ms(100);
      uart1_ngetc((unsigned char *) strbuf, 0, RX_BUF_SIZE, RX_BUF_SIZE);
      uart_printf("FONA reply %s\r\n", strbuf);
      uart_puts_P(PSTR("[SensorNode @ FONA ]$ "));
      strbuf_ptr = 0;
    }
  }
  uart_puts_P(PSTR("Exiting FONA shell\r\n"));
}

// TODO: clear settings and ideally put to sleep
void fona_destroy(Fona f) {
  if (f.pin_count != 3 || *f.port[0] != PORTD || *f.port[1] != PORTD ||
      f.reg_bit[0] != 2 || f.reg_bit[1] != 3) { // Rx1, TX1, RST
    uart_puts_P(PSTR(
          "Fona not initialized due to not being initialized on PD2 then PD3 then a third pin\r\n"));
    return;
  }
  // TODO: destroy uart setup both on fona and on sensornode

  //uart1_puts_P("AT+CIPSERVER=0\r\n"); // this just disconnects from the client
  //uart1_puts_P("AT+CIPCLOSE\r\n"); // this actually turns off the server
  //TODO: command to turn off fona or as close to that as possible
  //TODO: destroy uart1

  uart_puts_P(PSTR("fona Cleared of any settings\r\n"));
}
