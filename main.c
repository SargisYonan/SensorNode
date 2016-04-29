#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "dht.h"
#include "uart.h"
#include "parser.h"

#define UART_BAUD 19200
/*void
uart0_init(void){
#include<util/setbaud.h>
    UBRR0H=UBRRH_VALUE;
    UBRR0L=UBRRL_VALUE;
    UCSR0B|=_BV(TXEN0);
*no change to UCSR0B/C to get 8 bits/no parity/1 stop bit*
}*/
/*
void
uart0_putc(uint8_t c){
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}*/

static void
serial_init(void){
    uart_init(UART_BAUD_SELECT(UART_BAUD, F_CPU));
}


int
main(void){
    char buf[8]; /* character buffer for itoa */
    float hum, temp;
    struct dht22 d;
    unsigned int a = 0xFF01;
    //unsigned char b = 'b';
    serial_init();
    dht_init(&d);
    sei();
    uart_puts_P("Size of unsigned int ");
    uart_puts(itoa(sizeof(unsigned int), buf, 10));
    uart_puts_P("\r\n");
    uart_puts_P("TEST CAST unsigned int to uint8_t ");
    uart_puts(itoa((uint8_t) a, buf, 10));
    uart_puts_P("\r\n");
    uart_puts_P("Size of unsigned char ");
    uart_puts(itoa(sizeof(unsigned char), buf, 10));
    uart_puts_P("\r\n");
    uart_puts_P("DHT22 initialized...\r\n");
    uart_puts_P("Pin ");
    uart_puts(itoa(DHT_BIT, buf, 10));
    uart_puts("\r\n");

    for(;;){
       if(dht_read_data(&d, &temp, &hum)){
            uart_puts_P("Temperature ");
            sprintf_P(buf, PSTR("%0.2f"), temp);
            uart_puts(buf);
            uart_puts_P("C\t\tHumidity ");
            sprintf_P(buf, PSTR("%0.2f"), hum);
            uart_puts(buf);
            uart_puts_P("%\r\n\r\n");
        } else {
            uart_puts_P("FAILURE\r\n");
        }
        //process_uart();
        _delay_ms(2000);
    }
}
