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
#include "uart_macros.h"
#include "parser.h"

static void
serial_init(void){
#ifdef DEBUG
    DEBUG_INIT(UART_BAUD_SELECT(DEBUG_BAUD, F_CPU));
#endif
    RADIO_INIT(UART_BAUD_SELECT(RADIO_BAUD, F_CPU));
}

#define COMMAND_ERROR "BAD COMMAND\r\n"

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
#ifdef DEBUG
            DEBUG_PUTS_P("MAIN: parser_flags.command_error=1\r\n");
#endif
            uart_puts_P(COMMAND_ERROR);
        }
        if( parser_flags.measure_temperature &&
            parser_flags.measure_humidity ){
            dht_read_data( &d, &temp, &hum);
            sprintf_P(buf, PSTR("T=%0.2f:H=%0.2f\r\n"), temp, hum);
#ifdef DEBUG
            DEBUG_PUTS_P("MAIN: measure_temperature && measure_humidity\r\n");
            DEBUG_PUTS(buf);
#endif
            parser_flags.measure_temperature=0;
            parser_flags.measure_humidity=0;
        }
        else if( parser_flags.measure_temperature ){
            dht_read_data( &d, &temp, &hum);
            sprintf_P(buf, PSTR("T=%0.2f\r\n"), temp);
#ifdef DEBUG
            DEBUG_PUTS_P("MAIN: measure_temperature\r\n");
            DEBUG_PUTS(buf);
#endif

            parser_flags.measure_temperature=0;
        }
        else if( parser_flags.measure_humidity ){
            dht_read_data( &d, &temp, &hum);
            sprintf_P(buf, PSTR("H=%0.2f\r\n"), hum);
#ifdef DEBUG
            DEBUG_PUTS_P("MAIN: measure_humidity\r\n");
            DEBUG_PUTS(buf);
#endif
            parser_flags.measure_humidity=0;
        }
        if(buf[0]){
            RADIO_PUTS(buf);
            buf[0]='\0';
        }
    }
}
