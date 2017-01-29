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
#define RX_BUF_SIZE 16
#define TX_BUF_SIZE 16

// TODO: make this have non-blocking read/write

static volatile unsigned char RXbuf[RX_BUF_SIZE]; // circular buffer
static volatile uint8_t RXhead, RXtail;

ISR(USART0_RX_vect) {
  unsigned char data;
  //RXC0 = 0; // manually clear flag;
  if ((RXtail + 1) % RX_BUF_SIZE == RXhead) {
    data = UDR0; // throw away data if buffer full
    return;
  }
  RXbuf[RXtail] = UDR0;
  RXtail = (RXtail + 1) % RX_BUF_SIZE;
}

// function to initialize UART
void uart_init (void) {
  UBRR0H = (BAUD_PRESCALE >> 8) & 0xFF;	// get the upper 8 bits
  UBRR0L = BAUD_PRESCALE & 0xFF;  // get the lower 8 bits
  UCSR0B |= _BV(TXEN0) | _BV(RXEN0);	// enable receiver and transmitter
  UCSR0B |= _BV(RXCIE0); // enable receive interrupt
  UCSR0C |= _BV(UCSZ01) | _BV(UCSZ00); // 8 data bits

  RXhead = RXtail = 0; // start it at first location

  unsigned char data;
  while (UCSR0A & _BV(RXC0)) data = UDR0; // manual flush on startup
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
  if (RXhead == RXtail) return '\0'; // no new data
  unsigned char data = RXbuf[RXhead];
  RXhead = (RXhead + 1) % RX_BUF_SIZE;
  return data;
}
