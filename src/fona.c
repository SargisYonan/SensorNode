
#include "fona.h"

#include <avr/pgmspace.h>
#include "extra_util.h"

#include "uart.h"

#define FONARECEIVEMSG(MSG) uart_printf("Fona received message: %s\r\n", MSG)
#define FONAEXPECTRECEIVEMSG(MSG) uart_printf("Fona received expected message: %s\r\n", MSG)
#define FONANOTRECEIVEMSG(MSG) uart_printf("Fona didn't receive message: %s\r\n", MSG)

#define MILLISECOND (1)
#define ONESECOND (1000 * MILLISECOND)

#define SERVERPROTOCOL "TCP"
#define SERVERIP "arboretum-microgrid.ddns.net"
#define SERVERPORT "1234"

#define ATTEMPTS10 10

// will loop for characters "attempts" number of times or until receives a '\r'
// NOTE: will simply reset andoverwrite itself on overflow
// This function treats an "attempt" as an instance where 0 bytes are read
// as you might expect, the string is updates to strbuf based off strbuf_ptr
// there is no "error", this function simply ends by returning strbuf_ptr
uint8_t fonaGetData(char *strbuf, uint8_t strbuf_ptr, uint8_t attempts) {
  for (uint8_t i = 0; i < attempts;) {
    int16_t numBytes =
      (int16_t) uart1_ngetc(strbuf, strbuf_ptr, RX_BUF_SIZE, RX_BUF_SIZE);
    if (numBytes == -1) {
      strbuf[0] = '\0';
      strbuf_ptr = 0;
    } else {
      strbuf_ptr += numBytes;
    }
    if (numBytes == 0) {
      i++; // wasted an attempt
    }
    if (strbuf_ptr > 0 && strbuf[strbuf_ptr - 1]) { // got your string
      strbuf_ptr--; // update postition as you...
      strbuf[strbuf_ptr] = '\0'; // trim the leading carriage return
      break;
    }
  }
  return strbuf_ptr;
}

// your expected msg should expect a carriagereturn at the end to cut it off
// return 0 if no valid reply or 1 if valid reply given
uint8_t fonaWaitForReply(char *expectedmsg, int64_t milliseconds) {
  if (!expectedmsg) {
    FONANOTRECEIVEMSG(expectedmsg);
    return 0;
  }
  if (milliseconds > 32 * ONESECOND) milliseconds = 32 * ONESECOND; // 32sec max
  timer1_setCounter(0); // reset timer
  for (;;) {
    char strbuf[RX_BUF_SIZE] = "";
    uint8_t strbuf_ptr = 0;
    while (strbuf_ptr == 0 || strbuf[strbuf_ptr - 1] != '\r') {
      int16_t numbytes = (int16_t) uart1_ngetc(strbuf, strbuf_ptr, RX_BUF_SIZE,
          RX_BUF_SIZE);
      if (numbytes == -1) {
        uart_puts_P(PSTR("Fona reply was too many characters\r\n"));
        return 0;
      }
      strbuf_ptr += numbytes;

      if (timer1_readCounter() / 15.625 >= milliseconds) break;
    }
    if (timer1_readCounter()  / 15.625 >= milliseconds) break;

    char *found = strstr(strbuf, expectedmsg);
    strbuf[strbuf_ptr - 1] = '\0'; // remove the carriage return
    if (found) {
      FONAEXPECTRECEIVEMSG(strbuf);
      return 1;
    } else {
      FONARECEIVEMSG(strbuf);
    }
  }
  FONANOTRECEIVEMSG(expectedmsg);
  return 0;
}

uint8_t fonaAttemptConnection(void) {
  uart1_puts_P(PSTR("AT+CIPSTATUS\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIPSTATUS\r\n"));
  if (!fonaWaitForReply("IP STATUS\r", 2 * ONESECOND)) {
    uart_puts_P(PSTR(
          "FONA isn't in correct internal state to start a connection"));
    return 0;
  }

  uart1_printf("AT+CIPSTART=\"%s\",\"%s\",\"%s\"\r\n", SERVERPROTOCOL,
      SERVERIP, SERVERPORT);
  uart_printf("Sending to FONA: AT+CIPSTART=\"%s\",\"%s\",\"%s\"\r\n",
      SERVERPROTOCOL, SERVERIP, SERVERPORT);
  if (!fonaWaitForReply("CONNECT OK\r", 10 * ONESECOND)) {
    uart_puts_P("Connection timed out\r\n");
    return 0;
  }
  uart_printf("Connection using %s to %s with port %s was successful\r\n",
      SERVERPROTOCOL, SERVERIP, SERVERPORT);
  return 1;
}

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
  timer1_init();

  char strbuf[RX_BUF_SIZE];
  uart1_puts_P(PSTR("AT\r\n"));
  _delay_ms(100);
  uart1_ngetc(strbuf, 0, RX_BUF_SIZE, RX_BUF_SIZE); // clear newline
  _delay_ms(100);
  if(uart1_ngetc((unsigned char *) strbuf, 0, 20, RX_BUF_SIZE) == -1 || !strstr(strbuf, "OK")) {
    uart_printf("Error, invalid response from fona: %s\r\n", strbuf);
    return;
  }

  // SHUTOFF ALL GPRS COMMUNICATIONS
  uart1_puts_P(PSTR("AT+CIPSHUT\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIPSHUT\r\n"));
  if (!fonaWaitForReply("SHUT OK\r", 5 * ONESECOND)) {
    uart_printf("FONA not initialized correctly\r\n");
    return;
  }

  // Ensure that we are on initial IP STATUS STATE
  uart1_puts_P(PSTR("AT+CIPSTATUS\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIPSTATUS\r\n"));
  if (!fonaWaitForReply("IP INITIAL\r", 1 * ONESECOND)) {
    uart_printf("FONA not initialized correctly\r\n");
    return;
  }

  // Confirm APN, USERNAME, PASS (NOTE: you need not re-specify)
  uart1_puts_P(PSTR("AT+CSTT\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CSTT\r\n"));
  if (!fonaWaitForReply("OK\r", 5 * ONESECOND)) {
    uart_printf("FONA not initialized correctly\r\n");
    return;
  }

  // Enable GPRS connection (will now start blinking)
  uart1_puts_P(PSTR("AT+CIICR\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIICR\r\n"));
  if (!fonaWaitForReply("OK\r", 10 * ONESECOND)) {
    uart_printf("FONA not initialized correctly\r\n");
    return;
  }

  // Get IP (this is neccessary to advance internal state)
  uart1_puts_P(PSTR("AT+CIFSR\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIFSR\r\n"));
  _delay_ms(100); // this is a two part step technically so delay between TX
  uart1_puts_P(PSTR("AT+CIPSTATUS\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIPSTATUS\r\n"));
  if (!fonaWaitForReply("IP STATUS\r", 2 * ONESECOND)) {
    uart_printf("FONA not initialized correctly\r\n");
    return;
  }

  if (!fonaAttemptConnection()) {
    uart_puts_P(PSTR("FONA not initialized correctly"));
    return;
  }

  uart_puts_P(PSTR("Fona initialized\r\n"));
}

// read everything sent to uart for some number of attempts
void fona_read(Fona f) {
  char strbuf[RX_BUF_SIZE];
  uint8_t strbuf_ptr = 0;
  while (fonaGetData(strbuf, strbuf_ptr, ATTEMPTS10) != 0) {
    uart_printf("FONA received: %s\r\n", strbuf);
  }
  uart_puts_P(PSTR("Finished receiving messages from FONA\r\n"));
  uart_flushTX();
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
  // this is a special shell that runs when "-s" is passed as a string
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
      if (strbuf_ptr > 1) uart1_printf("%s\r\n", strbuf);
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

  uart1_puts_P(PSTR("AT+CIPSHUT\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIPSHUT\r\n"));
  if (!fonaWaitForReply("SHUT OK\r", 5 * ONESECOND)) {
    uart_printf("FONA not de-initialized correctly\r\n");
    return;
  }

  uart_puts_P(PSTR("fona Cleared of any settings\r\n"));
}
