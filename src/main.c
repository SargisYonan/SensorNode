#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>
#ifdef DHT_SENSOR
#include "dht.h"
#endif // DHT_SENSOR
#ifdef TEMP_SENSOR
#include "OneWire.h"
#endif // TEMP_SENSOR
#ifdef LIGHT_SENSOR
#include "I2C_lib.h"
#endif // LIGHT_SENSOR
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

#define LED PB7
#define LEDDDR DDRB
#define LEDPORT PORTB

int
main(void){
    char buf[16];
    uint32_t ledcount = 0; // counter to slow down LED access
    LEDDDR |= _BV(LED);
#ifdef DHT_SENSOR
    float hum, temp;
    struct dht22 d;
    dht_init(&d);
#endif // DHT_SENSOR
#ifdef TEMP_SENSOR
    float temp;
#endif // TEMP_SENSOR
#ifdef LIGHT_SENSOR
    uint16_t light;
    I2CInit();
#endif // LIGHT_SENSOR
    serial_init();
    sei();
#ifdef DEBUG
    DEBUG_PUTS_P("MAIN: Initialization complete, ready to recieve commands\r\n");
#endif
    for(;;){
        process_uart();
        if( parser_flags.command_recieved ){
            parser_flags.command_recieved=0;
            parse_command();
#ifdef DEBUG
            DEBUG_PUTS_P("MAIN: Command recieved, parsing...\r\n");
#endif
        }
        if( parser_flags.command_error ){
            /* send error message */
            parser_flags.command_error=0;
#ifdef DEBUG
            DEBUG_PUTS_P("MAIN: parser_flags.command_error=1\r\n");
#endif
            <<<<<<< HEAD
                RADIO_PUTS_P(COMMAND_ERROR);
        }
        =======
            uart_puts_P(COMMAND_ERROR);
    }
    >>>>>>> 5fd0172... Re-tabbed code
#ifdef DHT_SENSOR
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
#endif // DHT_SENSOR
#ifdef TEMP_SENSOR
    if( parser_flags.measure_temperature){
        temp = getTemperatureC();
        sprintf_P(buf, PSTR("T=%0.2f\r\n"), temp);
#ifdef DEBUG
        DEBUG_PUTS_P("MAIN: measure_temperature\r\n");
        DEBUG_PUTS(buf);
#endif
        parser_flags.measure_temperature=0;
    }
#endif //TEMP_SENSOR
#ifdef LIGHT_SENSOR
    if( parser_flags.measure_light){
        light = I2CReadValue();
        sprintf_P(buf, PSTR("L=%d\r\n"), (int) light);
#ifdef DEBUG
        DEBUG_PUTS_P("MAIN: measure_light\r\n");
        DEBUG_PUTS(buf);
#endif
        parser_flags.measure_light=0;
    }
#endif //LIGHT_SENSOR
#ifdef DHT_SENSOR
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
#endif //DHT_SENSOR
    else if (parser_flags.set_setpoint) {
        sprintf_P(buf, PSTR("set to: %d\r\n"), (int) parser_flags.var_setpoint);
#ifdef DEBUG
        DEBUG_PUTS_P("MAIN: set_setpoint\r\n");
        DEBUG_PUTS(buf);
#endif
        parser_flags.set_setpoint = 0;
    }
    else if (parser_flags.get_setpoint) {
        sprintf_P(buf, PSTR("S=%d\r\n"), (int) parser_flags.var_setpoint);
#ifdef DEBUG
        DEBUG_PUTS_P("MAIN: get_setpoint\r\n");
        DEBUG_PUTS(buf);
#endif
        parser_flags.get_setpoint = 0;
    }
    else if (parser_flags.command_error_setpoint) {
        sprintf_P(buf, PSTR("BAD COMMAND 1\r\n"));
#ifdef DEBUG
        DEBUG_PUTS_P("MAIN: parser_flags.command_error_setpoint=1\r\n");
#endif
        parser_flags.command_error_setpoint = 0;
    }
    if(buf[0]){
        RADIO_PUTS(buf);
        buf[0]='\0';
    }
    if (ledcount-- <= 0) {
        ledcount = F_CPU / 1e2;
#ifdef LIGHT_SENSOR
        light = I2CReadValue();
        if (light <= parser_flags.var_setpoint * 0.8) LEDPORT |= _BV(LED);
        else if (light >= parser_flags.var_setpoint * 1.2) LEDPORT &= ~_BV(LED);
#endif //LIGHT_SENSOR
    }
}
}
