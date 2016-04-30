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


static void
serial_init(void){
    uart_init(UART_BAUD_SELECT(UART_BAUD, F_CPU));
}


int
main(void){
    char buf[16];
    float hum, temp;
    struct dht22 d;
    serial_init();
    dht_init(&d);
    sei();
    for(;;){
        process_uart();
        if( parser_flags.command_recieved ){
            parser_flags.command_recieved=0;
            parse_command();
        }
        if( parser_flags.command_error ){
            /* send error message */
            parser_flags.command_error=0;
            uart_puts_P("BAD COMMAND\r\n");
        }
        if( parser_flags.measure_temperature &&
            parser_flags.measure_humidity ){
            dht_read_data( &d, &temp, &hum);
            sprintf_P(buf, PSTR("T=%0.2f:H=%0.2f\r\n"), temp, hum);
            parser_flags.measure_temperature=0;
            parser_flags.measure_humidity=0;
        }
        else if( parser_flags.measure_temperature ){
            dht_read_data( &d, &temp, &hum);
            sprintf_P(buf, PSTR("T=%0.2f\r\n"), temp);
            parser_flags.measure_temperature=0;
        }
        else if( parser_flags.measure_humidity ){
            dht_read_data( &d, &temp, &hum);
            sprintf_P(buf, PSTR("H=%0.2f\r\n"), hum);
            parser_flags.measure_humidity=0;
        }
        if(buf[0]){
            uart_puts(buf);
            buf[0]='\0';
        }
    }
}
