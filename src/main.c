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
  
  uart_init(19200);

  uart_puts("testing123\r\n");

  while (1) {
    unsigned char data = uart_getc();
    if (data == '\0') continue; // no data;
    uart_putc(data);
    if (data == '\r') uart_putc('\n');
  }
  return 0;
}
