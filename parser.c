#include <avr/io.h>
#include <stdint.h>
#include "parser.h"
#include "uart.h"
#include "uart_macros.h"
#include <string.h>

char uart_buffer[UART_RX_BUFFER_SIZE];
char parser_buffer[UART_RX_BUFFER_SIZE];
uint8_t pb_index=0;

void process_uart(void){
    uint16_t c;
    c = RADIO_GETC();
    if( (c & UART_FRAME_ERROR) ||
        (c & UART_OVERRUN_ERROR) ||
        (c & UART_BUFFER_OVERFLOW) ){
        /* set an error flag */
        parser_flags.uart_error = 1;
        /* should we reset the buffer? */
        return;
    }

    /* there's a uart character to process */
    if(!(c & UART_NO_DATA)){
        if (c == '\n'){
            // do we do something if pb_index = 0 at this point?
            // --> probably not since it will be parsed as a bad command by parse_command
            parser_flags.command_recieved=1;
            memcpy(parser_buffer, uart_buffer, pb_index);
            parser_buffer[pb_index]=0;
            pb_index=0;
            /* 
             * resetting the index should allow us to avoid 
             * memsetting the buffer 
             */
            return;
        }
        uart_buffer[pb_index++]= (unsigned char) c; /* lose those uart status flags */

        /* bound checking if we recieve way to many characters drop the buffer */
        if(pb_index >= UART_RX_BUFFER_SIZE){
            /* clear the buffer and send a command error */
            //parser_flags.command_error=1;
            pb_index=0;
        }
    }
}

void parse_command(void) {
    /* look at queries */
#ifdef DHT_SENSOR
    if(strcasestr(parser_buffer, "T?")){
        parser_flags.measure_temperature=1;
    }
    else if(strcasestr(parser_buffer, "H?")){
        parser_flags.measure_humidity=1;
    }
#endif /* DHT_SENSOR */
#ifdef TEMP_SENSOR
    if(strcasestr(parser_buffer, "T?")){
        parser_flags.measure_temperature=1;
    }
#endif /* TEMP_SENSOR */
#ifdef LIGHT_SENSOR
    if(strcasestr(parser_buffer, "L?")){
        parser_flags.measure_light=1;
    }
#endif /* LIGHT_SENSOR */
    else if(strcasestr(parser_buffer, "READ")){
#ifdef DHT_SENSOR
        parser_flags.measure_temperature=1;
        parser_flags.measure_humidity=1;
#endif /* DHT_SENSOR */
#ifdef TEMP_SENSOR
        parser_flags.measure_temperature=1; 
#endif /* TEMP_SENSOR */
#ifdef LIGHT_SENSOR
        parser_flags.measure_light=1;
#endif /* LIGHT_SENSOR */
    }
    else {
        parser_flags.command_error=1;
    }
    parser_buffer[0]='\0'; /* 
                            * make parser empty string after using it
                            * to prevent reparsing same command
                            */
    return;
}
