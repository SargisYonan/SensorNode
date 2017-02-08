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

#ifndef RX_BUF_SIZE
#define RX_BUF_SIZE 16
#endif

#ifndef TX_BUF_SIZE
#define TX_BUF_SIZE 256
#endif

static unsigned char RXbuf[RX_BUF_SIZE]; // circular buffer
static uint8_t RXhead, RXtail;

static unsigned char TXbuf[TX_BUF_SIZE]; // circular buffer
static uint8_t TXhead, TXtail;

ISR(USART0_RX_vect) {
  unsigned char data;
  UCSR0A &= ~_BV(RXC0); // manually clear flag;
  if ((RXtail + 1) % RX_BUF_SIZE == RXhead) {
    data = UDR0; // throw away data if buffer full
    return;
  }
  RXbuf[RXtail] = UDR0;
  RXtail = (RXtail + 1) % RX_BUF_SIZE;
}

ISR(USART0_UDRE_vect) { // If this interrupt is enabled, we still need to do TX
  UCSR0A &= ~_BV(UDRE0); // manually clear flag
  // main purpose is for handling head being right behind tail
  if ((TXhead + 1) % TX_BUF_SIZE == TXtail)
    UCSR0B &= ~_BV(UDRE0); // done, turn off interrupt
  else if (TXhead == TXtail) {
    UCSR0B &= ~_BV(UDRE0); // done, turn off interrupt
    return; // entered by mistake, already serviced
  }
  UDR0 = TXbuf[TXhead];
  TXhead = (TXhead + 1) % TX_BUF_SIZE;
}

// function to initialize UART
void uart_init (void) {
  UBRR0H = (BAUD_PRESCALE >> 8) & 0xFF;	// get the upper 8 bits
  UBRR0L = BAUD_PRESCALE & 0xFF;  // get the lower 8 bits
  UCSR0B |= _BV(TXEN0) | _BV(RXEN0);	// enable receiver and transmitter
  UCSR0B |= _BV(RXCIE0); // enable receive interrupt
  UCSR0C |= _BV(UCSZ01) | _BV(UCSZ00); // 8 data bits

  RXhead = RXtail = 0; // start it at first location
  TXhead = TXtail = 0; // start it at first location

  unsigned char data;
  while (UCSR0A & _BV(RXC0)) data = UDR0; // manual flush on startup
}

// function to send data
// TODO: inform user of inability to transmit
void uart_putc (unsigned char data) {
  UCSR0B |= _BV(UDRIE0); // service when data register is empty, enable ISR
  if ((TXtail + 1) % TX_BUF_SIZE == TXhead) return; // buffer full
  TXbuf[TXtail] = data;
  TXtail = (TXtail + 1) % TX_BUF_SIZE;
}

void uart_puts (unsigned char *str) {
  for (int i = 0; i < (int) strlen((char *) str); i++) uart_putc(str[i]);
}

unsigned char uart_getc (void) {
  if (RXhead == RXtail) return '\0'; // no new data
  unsigned char data = RXbuf[RXhead];
  RXhead = (RXhead + 1) % RX_BUF_SIZE;
  return data;
}
