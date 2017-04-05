#include "uart.h"

#define BAUDRATE 19200U
#define BAUD_PRESCALE (((F_CPU / (BAUDRATE * 16UL))) - 1)
#define FREQ 16e6 // atmega2560 has a 16MHz crystal

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
  data = UDR0; // compiler please be quiet
  RXbuf[RXtail] = data;
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
  return (void) data; // just another way to make the compiler be quiet
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

// NOTE: User is responsible for making sure that: str_size - start > n
// Also, if n > TX_BUF_SIZE, you will still read at most TX_BUF_SIZE bytes
// str is string to be filled from start, n is max chars to read
// will read either till n bytes, till '\r', or before '\0', whichever first
// will append '\0' for you
// RETURN: number of bytes read (not including '\0')
//TODO: more generic for other styles of line endings
uint16_t uart_ngetc (unsigned char *str, uint16_t start, uint16_t n,
    uint16_t buf_size_max) {
  uint16_t bytes_read = 0;
  if (n <= 0) return bytes_read; // why even...?
  if (n > TX_BUF_SIZE) n = TX_BUF_SIZE;
  unsigned char data;
  while ((data = uart_getc()) != '\0') {
    if (start >= buf_size_max) {
      return (uint16_t) -1; // overflow error
    }
    str[start++] = data;
    if (++bytes_read == n) break; // finished reading n bytes
    if (data == '\r') break; // end of the string
  }
  str[start] = '\0'; // add that NULL terminating char
  return bytes_read;
}

// returns next thing in queue or NULL char
unsigned char uart_rxpeak (void) {
  if (RXhead == RXtail) return '\0';
  return RXbuf[RXhead];
}

// feels good to be able to use printf directly
void uart_printf (char *fmt, ...) {
  unsigned char buffer[TX_BUF_SIZE];
  va_list args;
  va_start(args, fmt); // start var arg list
  // basically sprintf but args is variadic
  vsprintf((char *) buffer, fmt, args);
  va_end(args); // end var arg list
  uart_puts(buffer);
}

// block untill the TXbuffer has been written
void uart_flushTX() {
  UCSR0B &= ~_BV(UDRE0); // we will finish TX, turn off interrupt
  while (TXhead != TXtail) {
    while(!(UCSR0A & _BV(UDRE0))) {}
    UDR0 = TXbuf[TXhead];
    TXhead = (TXhead + 1) % TX_BUF_SIZE;
  }
}
