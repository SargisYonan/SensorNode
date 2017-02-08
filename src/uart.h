#ifndef _UART_H_
#define _UART_H_

void uart_init(void);

void uart_putc(unsigned char);

void uart_puts(unsigned char *);

unsigned char uart_getc(void);

#endif //_UART_H_
