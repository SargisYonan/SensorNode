#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include "parser.h"
#include "uart.h"
#include "uart_macros.h"
#include <string.h>
#include <math.h>

char uart_buffer[UART_RX_BUFFER_SIZE];
char parser_buffer[UART_RX_BUFFER_SIZE];
uint8_t pb_index = 0;

void process_uart(){
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
            parser_buffer[pb_index] = '\0';
#ifdef DEBUG
            DEBUG_PUTS_P("Process_Uart: detected newline character\r\n\tcommand: ");
            DEBUG_PUTS(parser_buffer);
            DEBUG_PUTS_P("\r\n");
#endif /* DEBUG */
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

void parse_command() {
    /* look at queries */

    uint8_t parsed = 0; /*
                           * a boolean flag for tracking whether we already recieved a command
                           * Note: it is important to consider the possibility
                           * of having a fragment of a command but not finding
                           * out that it isn't a command until we commit
                           * ourselves to the process of setting command flags
                           * appropriately.
                           * IE: if we got "dfsnDTT1?", observe that without
                           * this flag, we would have ended up having no command
                           * or worse, calling this a "BAD COMMAND"
                           */

    if(!parsed && strcasestr(parser_buffer, "I?")){
        parser_flags.get_info = 1;
        parsed = 1;
    }
#ifdef DHT_SENSOR
    if(!parsed && strcasestr(parser_buffer, "DT")){
        parsed = 1;
        if (strcasestr(parser_buffer, "DT0?")) { // we know which sensor through bitmasks
            parser_flags.measure_dht_temperature=1;
        } else if (strcasestr(parser_buffer, "DT1?")) {
            parser_flags.measure_dht_temperature=2;
        } else if (strcasestr(parser_buffer, "DT2?")) {
            parser_flags.measure_dht_temperature=4;
        } else if (strcasestr(parser_buffer, "DT3?")) {
            parser_flags.measure_dht_temperature=8;
        } else {
            parsed = 0;
        }
    }
    if(!parsed && strcasestr(parser_buffer, "DH")){
        parsed = 1;
        if (strcasestr(parser_buffer, "DH0?")) { // we know which sensor through bitmasks
            parser_flags.measure_dht_humidity=1;
        } else if (strcasestr(parser_buffer, "DH1?")) {
            parser_flags.measure_dht_humidity=2;
        } else if (strcasestr(parser_buffer, "DH2?")) {
            parser_flags.measure_dht_humidity=4;
        } else if (strcasestr(parser_buffer, "DH3?")) {
            parser_flags.measure_dht_humidity=8;
        } else {
            parsed = 0;
        }
    }
#endif /* DHT_SENSOR */
#ifdef TEMP_SENSOR
    if(!parsed && strcasestr(parser_buffer, "T")){
        parsed = 1;
        if (strcasestr(parser_buffer, "T0?")) { // we know which sensor through bitmasks
            parser_flags.measure_temperature=1;
        } else if (strcasestr(parser_buffer, "T1?")) {
            parser_flags.measure_temperature=2;
        } else if (strcasestr(parser_buffer, "T2?")) {
            parser_flags.measure_temperature=4;
        } else if (strcasestr(parser_buffer, "T3?")) {
            parser_flags.measure_temperature=8;
        } else {
            parsed = 0;
        }
    }
#endif /* TEMP_SENSOR */
#ifdef LIGHT_SENSOR
    if(!parsed && strcasestr(parser_buffer, "L")){
        parsed = 1;
        if (strcasestr(parser_buffer, "L0?")) { // we know which sensor through bitmasks
            parser_flags.measure_light=1;
        } else if (strcasestr(parser_buffer, "L1?")) {
            parser_flags.measure_light=2;
        } else {
            parsed = 0;
        }
    }
#endif /* LIGHT_SENSOR */
    if(!parsed && strcasestr(parser_buffer, "READ")){
        parser_flags.measure_all = 1;
        parsed = 1;
    }
    if (!parsed && strcasestr(parser_buffer, "S=")) {
        int i = 0;
        while (parser_buffer[i] != 'S' || parser_buffer[i + 1] != '=') {
            i++;
        }
        uint16_t tval = 0; //temporarily used for calculating given number
        int start = (i += 2); // move to index where number should start
        while(isdigit(parser_buffer[i])) {
            i++;
        }
        int stop = i - 1;
        if (stop < start) parser_flags.command_error_setpoint = 1; // if stop is less than start, no number was entered
        if (!parser_flags.command_error_setpoint) {
            parser_flags.set_setpoint = 1;
            parser_buffer[stop + 1] = '\0';
            tval = (uint16_t) atoi(start + parser_buffer); // convert only the number part of the string
            parser_flags.var_setpoint = tval;
        }
        parsed = 1;
    }
    if (!parsed && strcasestr(parser_buffer, "S?")) {
        parser_flags.get_setpoint=1;
        parsed = 1;
    }
    if (!parsed) {
        parser_flags.command_error=1;
        parsed = 1;
    }
    parser_buffer[0]='\0'; /*
                            * make parser empty string after using it
                            * to prevent reparsing same command
                            */
    return;
}
