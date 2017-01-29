#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>

#define BAUDRATE 19200U
#define BAUD_PRESCALE (((F_CPU / (BAUDRATE * 16UL))) - 1)
#define FREQ 16e6 // atmega2560 has a 16MHz crystal

// TODO: make this have non-blocking read/write

// function to initialize UART
void uart_init (void) {
  UBRR0H = (BAUD_PRESCALE >> 8) & 0xFF;	// get the upper 8 bits
  UBRR0L = BAUD_PRESCALE & 0xFF;  // get the lower 8 bits
  UCSR0B|= _BV(TXEN0) | _BV(RXEN0);	// enable receiver and transmitter
  // default frame format is fine so not set manually
}

// function to send data
void uart_putc (unsigned char data) {
  while (!( UCSR0A & _BV(UDRE0)));	// wait until register is free
  UDR0 = data;											// load data in the register
}

void uart_puts (char *str) {
  for (int i = 0; i < (int) strlen(str); i++) uart_putc(str[i]);
}

unsigned char uart_getc (void) {
  while (!(UCSR0A & (_BV(RXC0)))); // wait until register receives data
  return UDR0;
}
