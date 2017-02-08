#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>
#ifdef DHT_SENSOR
#include "dht.h"
#endif // DHT_SENSOR
#ifdef TEMP_SENSOR
#include "OneWire.h"
#endif // TEMP_SENSOR
#ifdef LIGHT_SENSOR
#include "I2C_lib.h"
#endif // LIGHT_SENSOR
#include "uart.h"

#define LED PB7
#define LEDDDR DDRB
#define LEDPORT PORTB

int main(void){
  sei();
  
  uart_init();

  unsigned char cmd[TX_BUF_SIZE + 1]; // max amount written by uart_ngetc()
  uint16_t cmd_index = 0;
  while (1) {
    // this way, it will point to the NULL character at the end
    // as well as not count it as a written index
    cmd_index += uart_ngetc(cmd, cmd_index, TX_BUF_SIZE);
    if (cmd_index > 0 && cmd[cmd_index - 1] == '\r') {
      uart_puts(cmd);
      uart_putc('\n');
      cmd_index = 0; // restart the index to start a new command
    }
    /*
    unsigned char data = uart_getc();
    if (data == '\0') continue; // no data;
    uart_putc(data);
    if (data == '\r') uart_putc('\n');
    */
  }
  return 0;
}
