#ifndef _UART_H_
#define _UART_H_

#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdarg.h>

#ifndef RX_BUF_SIZE
#define RX_BUF_SIZE 128
#endif

#ifndef TX_BUF_SIZE
#define TX_BUF_SIZE 256
#endif

void uart_init(void);

void uart_putc(unsigned char);

void uart_puts(unsigned char *);

void uart_puts_P(PGM_P);

unsigned char uart_getc(void);

uint16_t uart_ngetc(unsigned char *, uint16_t, uint16_t, uint16_t);

void uart_printf(char *, ...);

void uart_flushTX(void);

#endif //_UART_H_
