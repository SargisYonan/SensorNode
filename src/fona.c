
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
//#define SERVERIP "arboretum-microgrid.ddns.net"
#define SERVERIP "arboretum-backend.soe.ucsc.edu"
#define SERVERPORT "1234"

#define ATTEMPTS10 10

// will loop for characters "attempts" number of times or until receives a '\r'
// NOTE: will simply reset andoverwrite itself on overflow
// This function treats an "attempt" as an instance where 0 bytes are read
// as you might expect, the string is updates to strbuf based off strbuf_ptr
// there is no "error", this function simply ends by returning strbuf_ptr
uint8_t fonaGetData(char *strbuf, uint8_t strbuf_ptr, uint8_t attempts) {
  //uart3_flushTX(); // force flush of data to FONA
  for (uint8_t i = 0; i < attempts;) {
    int16_t numBytes =
      (int16_t) uart3_ngetc((unsigned char *) strbuf, strbuf_ptr, RX_BUF_SIZE,
          RX_BUF_SIZE);
    if (numBytes == -1) {
      strbuf[0] = '\0';
      strbuf_ptr = 0;
      break;
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
  if (milliseconds > 30 * ONESECOND) milliseconds = 30 * ONESECOND; // 30sec max
                                            // because experimentation > math
  timer1Reset(); // reset timer and millis
  for (;;) {
    char strbuf[RX_BUF_SIZE] = "";
    uint8_t strbuf_ptr = 0;
    while (strbuf_ptr == 0 || strbuf[strbuf_ptr - 1] != '\r') {
      int16_t numbytes = (int16_t) uart3_ngetc((unsigned char *) strbuf, strbuf_ptr, RX_BUF_SIZE,
          RX_BUF_SIZE);
      if (numbytes == -1) {
        uart_puts_P(PSTR("Fona reply was too many characters\r\n"));
        return 0;
      }
      strbuf_ptr += numbytes;

      if (timer1Millis() >= milliseconds) break;
    }
    if (timer1Millis() >= milliseconds) break;

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
  uart3_puts_P(PSTR("AT+CIPSTATUS\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIPSTATUS\r\n"));
  if (!fonaWaitForReply("IP STATUS\r", 10 * ONESECOND)) {
    uart_puts_P(PSTR(
          "FONA isn't in correct internal state to start a connection"));
    return 0;
  }

  uart3_printf("AT+CIPSTART=\"%s\",\"%s\",\"%s\"\r\n", SERVERPROTOCOL,
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
  uart3_init(4800);
  timer1_init();

  // Basically an initial test to make sure it responds at all
  uart3_puts_P(PSTR("AT\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT\r\n"));
  if(!fonaWaitForReply("OK\r", 20 * ONESECOND)) {
    uart_puts_P(PSTR("FONA not initialized correctly\r\n"));
    return;
  }

  // SHUTOFF ALL GPRS COMMUNICATIONS
  uart3_puts_P(PSTR("AT+CIPSHUT\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIPSHUT\r\n"));
  if (!fonaWaitForReply("SHUT OK\r", 20 * ONESECOND)) {
    uart_puts_P(PSTR("FONA not initialized correctly\r\n"));
    return;
  }

  // send the 0x1a instead
  /*
  // SET MESSAGES TO AUTOMATICALLY SEND AFTER AN AMOUNT OF SECONDS
  uart3_puts_P(PSTR("AT+CIPATS=1,2\r\n")); // enable time of 2 seconds
  uart_puts_P(PSTR("Sending to FONA: AT+CIPATS=1,2\r\n"));
  if (!fonaWaitForReply("OK\r", 5 * ONESECOND)) {
    uart_puts_P(PSTR("FONA not initialized correctly\r\n"));
    return;
  }
  */

  // Ensure that we are on initial IP STATUS STATE
  uart3_puts_P(PSTR("AT+CIPSTATUS\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIPSTATUS\r\n"));
  if (!fonaWaitForReply("IP INITIAL\r", 10 * ONESECOND)) {
    uart_puts_P(PSTR("FONA not initialized correctly\r\n"));
    return;
  }

  // Confirm APN, USERNAME, PASS (NOTE: you need not re-specify)
  uart3_puts_P(PSTR("AT+CSTT=\"wholesale\",\"\",\"\"\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CSTT\r\n"));
  if (!fonaWaitForReply("OK\r", 10 * ONESECOND)) {
    uart_puts_P(PSTR("FONA not initialized correctly\r\n"));
    return;
  }

  // Enable GPRS connection (will now start blinking)
  uart3_puts_P(PSTR("AT+CIICR\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIICR\r\n"));
  if (!fonaWaitForReply("OK\r", 10 * ONESECOND)) {
    uart_puts_P(PSTR("FONA not initialized correctly\r\n"));
    return;
  }

  // Get IP (this is neccessary to advance internal state)
  uart3_puts_P(PSTR("AT+CIFSR\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIFSR\r\n"));
  _delay_ms(100); // this is a two part step technically so delay between TX
  uart3_puts_P(PSTR("AT+CIPSTATUS\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIPSTATUS\r\n"));
  if (!fonaWaitForReply("IP STATUS\r", 2 * ONESECOND)) {
    uart_puts_P(PSTR("FONA not initialized correctly\r\n"));
    return;
  }

  if (!fonaAttemptConnection()) {
    uart_puts_P(PSTR("FONA not initialized correctly"));
    return;
  }

  uart_puts_P(PSTR("Fona initialized\r\n"));
}

// read everything sent to uart for some number of attempts
// TODO: make it connect, read some stuff, then disconnect
void fona_read(Fona f, char *read_data, uint16_t max_bytes) {
  if (f.write) {} // stop complaining
  char strbuf[RX_BUF_SIZE];
  uint8_t strbuf_ptr = 0;
  strbuf_ptr = fonaGetData(strbuf, strbuf_ptr, ATTEMPTS10);
  uart_printf("FONA received: %s\r\n", strbuf);
  snprintf(read_data, max_bytes, "%s\r\n", strbuf);
  uart_flushTX();
}

// TODO: make it connect, send some stuff, then disconnect
void fona_write(Fona f, char *str, uint16_t max_bytes) {
  if (f.write || max_bytes) {} // stop complaining
  // -s means shell mode
  if (strcmp(str, "-s") != 0) { // send to the server

    uart3_puts_P(PSTR("AT+CIPSEND\r\n"));
    uart_puts_P(PSTR("Sending to FONA: AT+CIPSEND\r\n"));
    if (!fonaWaitForReply("AT+CIPSEND\r", 10 * ONESECOND)) {
      uart_printf("FONA was unable to send message: %s\r\n", str);
    }

    uart3_printf("%s\r\n", str);
    uart_printf("Sending to FONA: %s\r\n", str);
    uart3_putc((unsigned char) 0x1a);
    uart_puts_P(PSTR("Sending End of Message to FONA\r\n"));
    if (!fonaWaitForReply("SEND OK\r", 5 * ONESECOND)) {
      uart_printf("FONA was unable to send message: %s\r\n", str);
    } else {
      uart_printf("FONA was successfully able to send message: %s\r\n", str);
    }

    uart_flushTX();
    return;
  }
  // this is a special shell that runs when "-s" is passed as a string
  char strbuf[RX_BUF_SIZE];
  uint8_t strbuf_ptr = 0;
  uart_puts_P(PSTR("[SensorNode @ FONA ]$ "));
  for (;;) {
    uint8_t bytes_read = uart_ngetc((unsigned char *) strbuf, strbuf_ptr, RX_BUF_SIZE, RX_BUF_SIZE);
    if (bytes_read == (uint8_t) -1) {
      strbuf_ptr = 0;
      uart_puts_P(PSTR("\r\nString cleared due to exceeding in size\r\n"));
      uart_puts_P(PSTR("[SensorNode @ FONA ]$ "));
      continue;
    }
    strbuf_ptr += bytes_read;
    if (strbuf_ptr > 0 && strbuf[strbuf_ptr - 1] == '\r') { // got the string
      if (strstr(strbuf, "-e")) break; // exit shell
      strbuf[strbuf_ptr - 1] = '\0';
      if (strbuf_ptr > 1) uart3_printf("%s\r\n", strbuf);
      _delay_ms(100);
      uart3_ngetc((unsigned char *) strbuf, 0, RX_BUF_SIZE, RX_BUF_SIZE); // clear newline
      uart_printf("FONA reply %s\r\n", strbuf);
      _delay_ms(100);
      uart3_ngetc((unsigned char *) strbuf, 0, RX_BUF_SIZE, RX_BUF_SIZE);
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

  uart3_puts_P(PSTR("AT+CIPSHUT\r\n"));
  uart_puts_P(PSTR("Sending to FONA: AT+CIPSHUT\r\n"));
  if (!fonaWaitForReply("SHUT OK\r", 10 * ONESECOND)) {
    uart_printf("FONA not de-initialized correctly\r\n");
    return;
  }

  uart_puts_P(PSTR("fona Cleared of any settings\r\n"));
}
