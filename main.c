#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "dht.h"

#define BAUD 19200
void
uart0_init(void){
#include<util/setbaud.h>
    UBRR0H=UBRRH_VALUE;
    UBRR0L=UBRRL_VALUE;
    UCSR0B|=_BV(TXEN0);
/*no change to UCSR0B/C to get 8 bits/no parity/1 stop bit*/
}

void
uart0_putc(uint8_t c){
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

FILE uart_dev = FDEV_SETUP_STREAM(uart0_putc, NULL, _FDEV_SETUP_WRITE);

int
main(void){
    float hum, temp;
    struct dht22 d;
    stdout = &uart_dev;
    uart0_init();
    dht_init(&d);
    printf(PSTR("DHT22 initialized...\r\n"));
    printf("Pin %d\r\n", BIT_DHT);
    for(;;){
        if(dht_read_data(&d, &temp, &hum)){
            printf_P(PSTR("SUCCESS\r\nTemperature %0.2f\r\nHumidity %0.2f\r\n"), temp, hum);
        } else {
            printf_P(PSTR("FAILURE\r\n"));
        }
        _delay_ms(2000);
    }
}
