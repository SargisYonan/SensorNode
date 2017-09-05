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

void uart_init(uint16_t);

void uart_putc(unsigned char);

void uart_puts(unsigned char *);

void uart_puts_P(PGM_P);

unsigned char uart_getc(void);

uint16_t uart_ngetc(unsigned char *, uint16_t, uint16_t, uint16_t);

void uart_printf(char *, ...);

void uart_flushTX(void);

#ifdef USING_UART1

void uart1_init(uint16_t);

void uart1_putc(unsigned char);

void uart1_puts(unsigned char *);

void uart1_puts_P(PGM_P);

unsigned char uart1_getc(void);

uint16_t uart1_ngetc(unsigned char *, uint16_t, uint16_t, uint16_t);

void uart1_printf(char *, ...);

void uart1_flushTX(void);

#endif // USING_UART1

#ifdef USING_UART2

void uart2_init(uint16_t);

void uart2_putc(unsigned char);

void uart2_puts(unsigned char *);

void uart2_puts_P(PGM_P);

unsigned char uart2_getc(void);

uint16_t uart2_ngetc(unsigned char *, uint16_t, uint16_t, uint16_t);

void uart2_printf(char *, ...);

void uart2_flushTX(void);

#endif // USING_UART2

#ifdef USING_UART3

void uart3_init(uint16_t);

void uart3_putc(unsigned char);

void uart3_puts(unsigned char *);

void uart3_puts_P(PGM_P);

unsigned char uart3_getc(void);

uint16_t uart3_ngetc(unsigned char *, uint16_t, uint16_t, uint16_t);

void uart3_printf(char *, ...);

void uart3_flushTX(void);

#endif // USING_UART3

#endif //_UART_H_
