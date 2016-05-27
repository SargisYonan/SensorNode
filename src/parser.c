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

int subindex(char *str, char *sub) { // returns the index of the end of the first occurence of sub in str or -1
    for (int i = 0; i < (int) strlen(str); i++) {
        int temp = i;
        for (int j = 0; j < (int) strlen(sub); j++) {
            if (j == (int) strlen(sub) - 1 && str[i] == sub[j]) return i; // if the last character is the same, then you found it
            if (str[i] == sub[j]) i++;
            else break;
        }
        i = temp;
    }
    return -1;
}

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

    uint8_t getVal = 0; /*
                         * a flag used to determine whether we should save a
                         * number in the flags buffer.
                         * Will be zero if unnecessary or contain the first
                         * index to read the number from
                         */

    if(!parsed && strcasestr(parser_buffer, "I?")){
        parser_flags.get_info = 1;
        parsed = 1;
    }
    if (!parsed && strcasestr(parser_buffer, "AT")) { // sensor status put here to acoid collision with Tx?
        parsed = 1;
        if (strcasestr(parser_buffer, "AT0?")) {
            parser_flags.get_actuator_status = _BV(0);
        }
        else if (strcasestr(parser_buffer, "AT1?")) {
            parser_flags.get_actuator_status = _BV(1);
        }
        else if (strcasestr(parser_buffer, "AT2?")) {
            parser_flags.get_actuator_status = _BV(2);
        }
        else if (strcasestr(parser_buffer, "AT3?")) {
            parser_flags.get_actuator_status = _BV(3);
        }
        else if (strcasestr(parser_buffer, "AT4?")) {
            parser_flags.get_actuator_status = _BV(4);
        }
        else if (strcasestr(parser_buffer, "AT5?")) {
            parser_flags.get_actuator_status = _BV(5);
        }
        else if (strcasestr(parser_buffer, "AT6?")) {
            parser_flags.get_actuator_status = _BV(6);
        }
        else if (strcasestr(parser_buffer, "AT7?")) {
            parser_flags.get_actuator_status = _BV(7);
        }
        if (!parser_flags.get_actuator_status) parsed = 0;
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
    if (!parsed && strcasestr(parser_buffer, "AO")) { // choosing whether or not the actuator is off
        parsed = 1;
        if (strcasestr(parser_buffer, "AO0")) {
            if (strcasestr(parser_buffer, "AO0?")) parser_flags.get_actuator_onoff = _BV(0);
            else if(strcasestr(parser_buffer, "AO0=")) {
                parser_flags.set_actuator_onoff = _BV(0);
                getVal = subindex(parser_buffer, "AO0=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AO1")) {
            if (strcasestr(parser_buffer, "AO1?")) parser_flags.get_actuator_onoff = _BV(1);
            else if(strcasestr(parser_buffer, "AO1=")) {
                parser_flags.set_actuator_onoff = _BV(1);
                getVal = subindex(parser_buffer, "AO1=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AO2")) {
            if (strcasestr(parser_buffer, "AO2?")) parser_flags.get_actuator_onoff = _BV(2);
            else if(strcasestr(parser_buffer, "AO2=")) {
                parser_flags.set_actuator_onoff = _BV(2);
                getVal = subindex(parser_buffer, "AO2=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AO3")) {
            if (strcasestr(parser_buffer, "AO3?")) parser_flags.get_actuator_onoff = _BV(3);
            else if(strcasestr(parser_buffer, "AO3=")) {
                parser_flags.set_actuator_onoff = _BV(3);
                getVal = subindex(parser_buffer, "A03=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AO4")) {
            if (strcasestr(parser_buffer, "AO4?")) parser_flags.get_actuator_onoff = _BV(4);
            else if(strcasestr(parser_buffer, "AO4=")) {
                parser_flags.set_actuator_onoff = _BV(4);
                getVal = subindex(parser_buffer, "AO4=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AO5")) {
            if (strcasestr(parser_buffer, "AO5?")) parser_flags.get_actuator_onoff = _BV(5);
            else if(strcasestr(parser_buffer, "AO5=")) {
                parser_flags.set_actuator_onoff = _BV(5);
                getVal = subindex(parser_buffer, "AO5=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AO6")) {
            if (strcasestr(parser_buffer, "AO6?")) parser_flags.get_actuator_onoff = _BV(6);
            else if(strcasestr(parser_buffer, "AO6=")) {
                parser_flags.set_actuator_onoff = _BV(6);
                getVal = subindex(parser_buffer, "AO6=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AO7")) {
            if (strcasestr(parser_buffer, "AO7?")) parser_flags.get_actuator_onoff = _BV(7);
            else if(strcasestr(parser_buffer, "AO7=")) {
                parser_flags.set_actuator_onoff = _BV(7);
                getVal = subindex(parser_buffer, "AO7=") + 1;
            }
        }
        if (!parser_flags.set_actuator_onoff && !parser_flags.get_actuator_onoff) parsed = 0;
    }
    if (!parsed && strcasestr(parser_buffer, "AA")) { // choosing whether actuator is forced on
        parsed = 1;
        if (strcasestr(parser_buffer, "AA0")) {
            if (strcasestr(parser_buffer, "AA0?")) parser_flags.get_actuator_armdisarm = _BV(0);
            else if(strcasestr(parser_buffer, "AA0=")) {
                parser_flags.set_actuator_armdisarm = _BV(0);
                getVal = subindex(parser_buffer, "AA0=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AA1")) {
            if (strcasestr(parser_buffer, "AA1?")) parser_flags.get_actuator_armdisarm = _BV(1);
            else if(strcasestr(parser_buffer, "AA1=")) {
                parser_flags.set_actuator_armdisarm = _BV(1);
                getVal = subindex(parser_buffer, "AA1=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AA2")) {
            if (strcasestr(parser_buffer, "AA2?")) parser_flags.get_actuator_armdisarm = _BV(2);
            else if(strcasestr(parser_buffer, "AA2=")) {
                parser_flags.set_actuator_armdisarm = _BV(2);
                getVal = subindex(parser_buffer, "AA2=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AA3")) {
            if (strcasestr(parser_buffer, "AA3?")) parser_flags.get_actuator_armdisarm = _BV(3);
            else if(strcasestr(parser_buffer, "AA3=")) {
                parser_flags.set_actuator_armdisarm = _BV(3);
                getVal = subindex(parser_buffer, "AA3=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AA4")) {
            if (strcasestr(parser_buffer, "AA4?")) parser_flags.get_actuator_armdisarm = _BV(4);
            else if(strcasestr(parser_buffer, "AA4=")) {
                parser_flags.set_actuator_armdisarm = _BV(4);
                getVal = subindex(parser_buffer, "AA4=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AA5")) {
            if (strcasestr(parser_buffer, "AA5?")) parser_flags.get_actuator_armdisarm = _BV(5);
            else if(strcasestr(parser_buffer, "AA5=")) {
                parser_flags.set_actuator_armdisarm = _BV(5);
                getVal = subindex(parser_buffer, "AA5=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AA6")) {
            if (strcasestr(parser_buffer, "AA6?")) parser_flags.get_actuator_armdisarm = _BV(6);
            else if(strcasestr(parser_buffer, "AA6=")) {
                parser_flags.set_actuator_armdisarm = _BV(6);
                getVal = subindex(parser_buffer, "AA6=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AA7")) {
            if (strcasestr(parser_buffer, "AA7?")) parser_flags.get_actuator_armdisarm = _BV(7);
            else if(strcasestr(parser_buffer, "AA7=")) {
                parser_flags.set_actuator_armdisarm = _BV(7);
                getVal = subindex(parser_buffer, "AA7=") + 1;
            }
        }
        if (!parser_flags.set_actuator_armdisarm && !parser_flags.get_actuator_armdisarm) parsed = 0;

    }
    if (!parsed && strcasestr(parser_buffer, "AP")) { // choosing setpoint for actuator
        parsed = 1;
        if (strcasestr(parser_buffer, "AP0")) {
            if (strcasestr(parser_buffer, "AP0?")) parser_flags.get_actuator_setpoint = _BV(0);
            else if(strcasestr(parser_buffer, "AP0=")) {
                parser_flags.set_actuator_setpoint = _BV(0);
                getVal = subindex(parser_buffer, "AP0=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AP1")) {
            if (strcasestr(parser_buffer, "AP1?")) parser_flags.get_actuator_setpoint = _BV(1);
            else if(strcasestr(parser_buffer, "AP1=")) {
                parser_flags.set_actuator_setpoint = _BV(1);
                getVal = subindex(parser_buffer, "AP1=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AP2")) {
            if (strcasestr(parser_buffer, "AP2?")) parser_flags.get_actuator_setpoint = _BV(2);
            else if(strcasestr(parser_buffer, "AP2=")) {
                parser_flags.set_actuator_setpoint = _BV(2);
                getVal = subindex(parser_buffer, "AP2=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AP3")) {
            if (strcasestr(parser_buffer, "AP3?")) parser_flags.get_actuator_setpoint = _BV(3);
            else if(strcasestr(parser_buffer, "AP3=")) {
                parser_flags.set_actuator_setpoint = _BV(3);
                getVal = subindex(parser_buffer, "AP3=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AP4")) {
            if (strcasestr(parser_buffer, "AP4?")) parser_flags.get_actuator_setpoint = _BV(4);
            else if(strcasestr(parser_buffer, "AP4=")) {
                parser_flags.set_actuator_setpoint = _BV(4);
                getVal = subindex(parser_buffer, "AP4=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AP5")) {
            if (strcasestr(parser_buffer, "AP5?")) parser_flags.get_actuator_setpoint = _BV(5);
            else if(strcasestr(parser_buffer, "AP5=")) {
                parser_flags.set_actuator_setpoint = _BV(5);
                getVal = subindex(parser_buffer, "AP5=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AP6")) {
            if (strcasestr(parser_buffer, "AP6?")) parser_flags.get_actuator_setpoint = _BV(6);
            else if(strcasestr(parser_buffer, "AP6=")) {
                parser_flags.set_actuator_setpoint = _BV(6);
                getVal = subindex(parser_buffer, "AP6=") + 1;
            }
        }
        else if (strcasestr(parser_buffer, "AP7")) {
            if (strcasestr(parser_buffer, "AP7?")) parser_flags.get_actuator_setpoint = _BV(7);
            else if(strcasestr(parser_buffer, "AP7=")) {
                parser_flags.set_actuator_setpoint = _BV(7);
                getVal = subindex(parser_buffer, "AP7=") + 1;
            }
        }
        if (!parser_flags.set_actuator_setpoint && !parser_flags.get_actuator_setpoint) parsed = 0;

    }
    if (!parsed && strcasestr(parser_buffer, "AS")) { // choosing sensor for actuator
        parsed = 1;
        if (strcasestr(parser_buffer, "AS0")) {
            if (strcasestr(parser_buffer, "AS0?")) parser_flags.get_actuator_choosesensor = _BV(0);
            else if(strcasestr(parser_buffer, "AS0=")) {
                parser_flags.set_actuator_choosesensor = _BV(0);
                getVal = subindex(parser_buffer, "AS0="); // this will cause it
                // to have a syntax error
                // because the start
                // index will point to
                // a non-integer (if
                // getVal isn't set
                // back to 0)
                if (isupper(parser_buffer[getVal + 1]) && isdigit(parser_buffer[getVal + 2]) &&
                        !isdigit(parser_buffer[getVal + 3])) {
                    char tempc = parser_buffer[getVal + 1];
                    int tempi = (int) parser_buffer[getVal + 2] - 48;
                    // 2 light sensors, 4 dht sensors, 4 temp sensors at max
                    if ((tempc == 'L' && tempi < 2) || (tempc == 'D' && tempi < 4) || (tempc == 'T' && tempi < 4)) {
                        parser_flags.value_buffer = (tempc == 'L' ? 0 : (tempc == 'D' ? 2 : 6)) + tempi;
                    } else {
                        parser_flags.set_actuator_choosesensor = 0; // this command is no longer valid
                        parser_flags.command_error_syntax = 1; // give it a syntax error
                    }
                    getVal = 0;
                }
            }
        }
        else if (strcasestr(parser_buffer, "AS1")) {
            if (strcasestr(parser_buffer, "AS1?")) parser_flags.get_actuator_choosesensor = _BV(1);
            else if(strcasestr(parser_buffer, "AS1=")) {
                parser_flags.set_actuator_choosesensor = _BV(1);
                getVal = subindex(parser_buffer, "AS1="); // this will cause it
                // to have a syntax error
                // because the start
                // index will point to
                // a non-integer (if
                // getVal isn't set
                // back to 0)
                if (isupper(parser_buffer[getVal + 1]) && isdigit(parser_buffer[getVal + 2]) &&
                        !isdigit(parser_buffer[getVal + 3])) {
                    char tempc = parser_buffer[getVal + 1];
                    int tempi = (int) parser_buffer[getVal + 2] - 48;
                    // 2 light sensors, 4 dht sensors, 4 temp sensors at max
                    if ((tempc == 'L' && tempi < 2) || (tempc == 'D' && tempi < 4) || (tempc == 'T' && tempi < 4)) {
                        parser_flags.value_buffer = (tempc == 'L' ? 0 : (tempc == 'D' ? 2 : 6)) + tempi;
                    } else {
                        parser_flags.set_actuator_choosesensor = 0; // this command is no longer valid
                        parser_flags.command_error_syntax = 1; // give it a syntax error
                    }
                    getVal = 0;
                }
            }
        }
        else if (strcasestr(parser_buffer, "AS2")) {
            if (strcasestr(parser_buffer, "AS2?")) parser_flags.get_actuator_choosesensor = _BV(2);
            else if(strcasestr(parser_buffer, "AS2=")) {
                parser_flags.set_actuator_choosesensor = _BV(2);
                getVal = subindex(parser_buffer, "AS2="); // this will cause it
                // to have a syntax error
                // because the start
                // index will point to
                // a non-integer (if
                // getVal isn't set
                // back to 0)
                if (isupper(parser_buffer[getVal + 1]) && isdigit(parser_buffer[getVal + 2]) &&
                        !isdigit(parser_buffer[getVal + 3])) {
                    char tempc = parser_buffer[getVal + 1];
                    int tempi = (int) parser_buffer[getVal + 2] - 48;
                    // 2 light sensors, 4 dht sensors, 4 temp sensors at max
                    if ((tempc == 'L' && tempi < 2) || (tempc == 'D' && tempi < 4) || (tempc == 'T' && tempi < 4)) {
                        parser_flags.value_buffer = (tempc == 'L' ? 0 : (tempc == 'D' ? 2 : 6)) + tempi;
                    } else {
                        parser_flags.set_actuator_choosesensor = 0; // this command is no longer valid
                        parser_flags.command_error_syntax = 1; // give it a syntax error
                    }
                    getVal = 0;
                }
            }
        }
        else if (strcasestr(parser_buffer, "AS3")) {
            if (strcasestr(parser_buffer, "AS3?")) parser_flags.get_actuator_choosesensor = _BV(3);
            else if(strcasestr(parser_buffer, "AS3=")) {
                parser_flags.set_actuator_choosesensor = _BV(3);
                getVal = subindex(parser_buffer, "AS3="); // this will cause it
                // to have a syntax error
                // because the start
                // index will point to
                // a non-integer (if
                // getVal isn't set
                // back to 0)
                if (isupper(parser_buffer[getVal + 1]) && isdigit(parser_buffer[getVal + 2]) &&
                        !isdigit(parser_buffer[getVal + 3])) {
                    char tempc = parser_buffer[getVal + 1];
                    int tempi = (int) parser_buffer[getVal + 2] - 48;
                    // 2 light sensors, 4 dht sensors, 4 temp sensors at max
                    if ((tempc == 'L' && tempi < 2) || (tempc == 'D' && tempi < 4) || (tempc == 'T' && tempi < 4)) {
                        parser_flags.value_buffer = (tempc == 'L' ? 0 : (tempc == 'D' ? 2 : 6)) + tempi;
                    } else {
                        parser_flags.set_actuator_choosesensor = 0; // this command is no longer valid
                        parser_flags.command_error_syntax = 1; // give it a syntax error
                    }
                    getVal = 0;
                }
            }
        }
        else if (strcasestr(parser_buffer, "AS4")) {
            if (strcasestr(parser_buffer, "AS4?")) parser_flags.get_actuator_choosesensor = _BV(4);
            else if(strcasestr(parser_buffer, "AS4=")) {
                parser_flags.set_actuator_choosesensor = _BV(4);
                getVal = subindex(parser_buffer, "AS4="); // this will cause it
                // to have a syntax error
                // because the start
                // index will point to
                // a non-integer (if
                // getVal isn't set
                // back to 0)
                if (isupper(parser_buffer[getVal + 1]) && isdigit(parser_buffer[getVal + 2]) &&
                        !isdigit(parser_buffer[getVal + 3])) {
                    char tempc = parser_buffer[getVal + 1];
                    int tempi = (int) parser_buffer[getVal + 2] - 48;
                    // 2 light sensors, 4 dht sensors, 4 temp sensors at max
                    if ((tempc == 'L' && tempi < 2) || (tempc == 'D' && tempi < 4) || (tempc == 'T' && tempi < 4)) {
                        parser_flags.value_buffer = (tempc == 'L' ? 0 : (tempc == 'D' ? 2 : 6)) + tempi;
                    } else {
                        parser_flags.set_actuator_choosesensor = 0; // this command is no longer valid
                        parser_flags.command_error_syntax = 1; // give it a syntax error
                    }
                    getVal = 0;
                }
            }
        }
        else if (strcasestr(parser_buffer, "AS5")) {
            if (strcasestr(parser_buffer, "AS5?")) parser_flags.get_actuator_choosesensor = _BV(5);
            else if(strcasestr(parser_buffer, "AS5=")) {
                parser_flags.set_actuator_choosesensor = _BV(5);
                getVal = subindex(parser_buffer, "AS5="); // this will cause it
                // to have a syntax error
                // because the start
                // index will point to
                // a non-integer (if
                // getVal isn't set
                // back to 0)
                if (isupper(parser_buffer[getVal + 1]) && isdigit(parser_buffer[getVal + 2]) &&
                        !isdigit(parser_buffer[getVal + 3])) {
                    char tempc = parser_buffer[getVal + 1];
                    int tempi = (int) parser_buffer[getVal + 2] - 48;
                    // 2 light sensors, 4 dht sensors, 4 temp sensors at max
                    if ((tempc == 'L' && tempi < 2) || (tempc == 'D' && tempi < 4) || (tempc == 'T' && tempi < 4)) {
                        parser_flags.value_buffer = (tempc == 'L' ? 0 : (tempc == 'D' ? 2 : 6)) + tempi;
                    } else {
                        parser_flags.set_actuator_choosesensor = 0; // this command is no longer valid
                        parser_flags.command_error_syntax = 1; // give it a syntax error
                    }
                    getVal = 0;
                }
            }
        }
        else if (strcasestr(parser_buffer, "AS6")) {
            if (strcasestr(parser_buffer, "AS6?")) parser_flags.get_actuator_choosesensor = _BV(6);
            else if(strcasestr(parser_buffer, "AS6=")) {
                parser_flags.set_actuator_choosesensor = _BV(6);
                getVal = subindex(parser_buffer, "AS6="); // this will cause it
                // to have a syntax error
                // because the start
                // index will point to
                // a non-integer (if
                // getVal isn't set
                // back to 0)
                if (isupper(parser_buffer[getVal + 1]) && isdigit(parser_buffer[getVal + 2]) &&
                        !isdigit(parser_buffer[getVal + 3])) {
                    char tempc = parser_buffer[getVal + 1];
                    int tempi = (int) parser_buffer[getVal + 2] - 48;
                    // 2 light sensors, 4 dht sensors, 4 temp sensors at max
                    if ((tempc == 'L' && tempi < 2) || (tempc == 'D' && tempi < 4) || (tempc == 'T' && tempi < 4)) {
                        parser_flags.value_buffer = (tempc == 'L' ? 0 : (tempc == 'D' ? 2 : 6)) + tempi;
                    } else {
                        parser_flags.set_actuator_choosesensor = 0; // this command is no longer valid
                        parser_flags.command_error_syntax = 1; // give it a syntax error
                    }
                    getVal = 0;
                }
            }
        }
        else if (strcasestr(parser_buffer, "AS7")) {
            if (strcasestr(parser_buffer, "AS7?")) parser_flags.get_actuator_choosesensor = _BV(7);
            else if(strcasestr(parser_buffer, "AS7=")) {
                parser_flags.set_actuator_choosesensor = _BV(7);
                getVal = subindex(parser_buffer, "AS7="); // this will cause it
                // to have a syntax error
                // because the start
                // index will point to
                // a non-integer (if
                // getVal isn't set
                // back to 0)
                if (isupper(parser_buffer[getVal + 1]) && isdigit(parser_buffer[getVal + 2]) &&
                        !isdigit(parser_buffer[getVal + 3])) {
                    char tempc = parser_buffer[getVal + 1];
                    int tempi = (int) parser_buffer[getVal + 2] - 48;
                    // 2 light sensors, 4 dht sensors, 4 temp sensors at max
                    if ((tempc == 'L' && tempi < 2) || (tempc == 'D' && tempi < 4) || (tempc == 'T' && tempi < 4)) {
                        parser_flags.value_buffer = (tempc == 'L' ? 0 : (tempc == 'D' ? 2 : 6)) + tempi;
                    } else {
                        parser_flags.set_actuator_choosesensor = 0; // this command is no longer valid
                        parser_flags.command_error_syntax = 1; // give it a syntax error
                    }
                    getVal = 0;
                }
            }
        }
        if (!parser_flags.set_actuator_choosesensor && !parser_flags.get_actuator_choosesensor) parsed = 0;
    }
    if (!parsed) {
        parser_flags.command_error=1;
        parsed = 1;
    }
    if (getVal) {
        //int i = 0;
        //while (parser_buffer[i] != 'S' || parser_buffer[i + 1] != '=') {
        //    i++;
        //}
        uint16_t tval = 0; //temporarily used for calculating given number
        //int start = (i += 2); // move to index where number should start
        int i = getVal;
        while(isdigit(parser_buffer[i])) {
            i++;
        }
        int stop = i - 1;
        if (stop < getVal) parser_flags.command_error_syntax = 1; // if stop is less than start, no number was entered
        if (!parser_flags.command_error_syntax) {
            parser_buffer[stop + 1] = '\0';
            tval = (uint16_t) atoi(getVal + parser_buffer); // convert only the number part of the string
            parser_flags.value_buffer = tval;
        }
    }
    getVal = 0;
    parser_buffer[0]='\0'; /*
                            * make parser empty string after using it
                            * to prevent reparsing same command
                            */
    return;
}
