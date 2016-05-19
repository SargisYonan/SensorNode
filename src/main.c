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

void S_RADIO_PUTS(char *buf) {
    char temp[strlen(buf + 1)]; // we don't want to modify the original array
    char temp2[16] = {}; // this is what happens when the compiler doesn't like pointer arithmetic
    strncpy(temp, buf, strlen(buf) + 1);
    int i;
    for (i = 0;; i += 15) { // print every 15 characters
        if (strlen(temp) - i > 15) {
            char tempc = temp[15 + i];
            temp[15 + i] = '\0';
            sprintf_P(temp2, PSTR("%s"), temp + i);
            //temp2[15] = '\0';
            RADIO_PUTS(temp2);
            temp[15 + i] = tempc;
        } else {
            break;
        }
    }
    //print the remaining characters
    //sprintf_P(temp2, PSTR("%s"), temp + i);
    //temp2[15] = '\0';
    //RADIO_PUTS(temp2);
}


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
    char buf[128];
    buf[0] = '\0';
    uint32_t ledcount = 0; // counter to slow down LED access
    LEDDDR |= _BV(LED);
#ifdef DHT_SENSOR
    float dht_hum, dht_temp;
    struct dht22 d;
    dht_init(&d);
#endif // DHT_SENSOR
#ifdef TEMP_SENSOR
    float temp;
    getTemperatureC(); // first read is garbage data
#endif // TEMP_SENSOR
#ifdef LIGHT_SENSOR
    uint16_t light;
    I2CInit();
#endif // LIGHT_SENSOR
    serial_init();
    sei();
    if (LIGHT_SENSOR_COUNT <= 0 && DHT_SENSOR_COUNT <= 0 && TEMP_SENSOR_COUNT <= 0) {
        RADIO_PUTS_P("No sensors installed, operation will now halt\r\n");
        for(;;);
    }
    //int current_sensor = -1; // keep track of which sensor we are currently handling
    uint8_t sensor_activated[MAX_SENSOR_COUNT] = {}; /*
                                                      * Tells which sensors are installed
                                                      * [1:0]: Light sensors
                                                      * [5:2]: DHT Sensors
                                                      * [9:6]: Temp Sensors
                                                      */
    for (int i = 0; i < LIGHT_SENSOR_COUNT && i < 2; i++) sensor_activated[i] = 1; // validate light sensors
    for (int i = 2; i < DHT_SENSOR_COUNT + 2 && i < 6; i++) sensor_activated[i] = 1; // validate humidity sensors
    for (int i = 6; i < TEMP_SENSOR_COUNT + 6 && i < 10; i++) sensor_activated[i] = 1; // validate temperature sensors
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
            RADIO_PUTS_P(COMMAND_ERROR);
        }
        else if (parser_flags.get_info) {
            sprintf(buf, "LC=%d DC=%d TC=%d\r\n",
                    LIGHT_SENSOR_COUNT, DHT_SENSOR_COUNT, TEMP_SENSOR_COUNT);
            S_RADIO_PUTS(buf);
            buf[0] = '\0';
            parser_flags.get_info = 0;
        }
        else if (parser_flags.measure_all) {
            char strlight[64] = "";
            char strdht[64] = "";
            char strtemp[64] = "";
            char str[64] = "";
#ifdef LIGHT_SENSOR
            for (int i = 0; i < 2 && sensor_activated[i]; i++) {
                light = I2CReadValue();
                sprintf(str, "L%d=%d ", i, light);
                strcat(strlight, str);
            }
#endif // LIGHT_SENSOR
#ifdef DHT_SENSOR
            for (int i = 2; i < 6 && sensor_activated[i]; i++) {
                dht_read_data(&d, &dht_temp, &dht_hum);
                sprintf(str, "DT%d=%0.2f DH%d=%0.2f ", i - 2, dht_temp, i - 2, dht_hum);
                strcat(strdht, dtr);
            }
#endif // DHT_SENSOR
#ifdef TEMP_SENSOR
            for (int i = 6; i < 10 && sensor_activated[i]; i++) {
                temp = getTemperatureC();
                sprintf(str, "T%d=%0.2f ", i - 6, temp);
                strcat(strtemp, str);
            }
#endif // TEMP_SENSOR
            sprintf(buf, "%s%s%s\r\n", strlight, strdht, strtemp);
            S_RADIO_PUTS(buf);
            buf[0] = '\0';
            parser_flags.measure_all = 0;
        }
#ifdef TEMP_SENSOR
        else if( parser_flags.measure_temperature){
            int bm = parser_flags.measure_temperature; // bitmask
            int sensor = (bm & 1 ? 1 : (bm & 2 ? 2 : (bm & 4 ? 3 : 4))); // which sensor?
            if (!sensor_activated[sensor + 5]) RADIO_PUTS_P(COMMAND_ERROR); // indexes 6-9
            else {
                temp = getTemperatureC();
                sprintf_P(buf, PSTR("T%d=%0.2f\r\n"), sensor - 1, temp);
#ifdef DEBUG
                DEBUG_PUTS_P("MAIN: measure_temperature\r\n");
                DEBUG_PUTS(buf);
#endif
            }
            parser_flags.measure_temperature=0;
        }
#endif //TEMP_SENSOR
#ifdef LIGHT_SENSOR
        else if( parser_flags.measure_light){
            int bm = parser_flags.measure_light; // bitmask
            int sensor = (bm & 1 ? 1 : 2); // which sensor?
            if (!sensor_activated[sensor - 1]) RADIO_PUTS_P(COMMAND_ERROR); // indexes 0-1
            else {
                light = I2CReadValue();
                sprintf_P(buf, PSTR("L%d=%d\r\n"), sensor - 1, (int) light);
#ifdef DEBUG
                DEBUG_PUTS_P("MAIN: measure_light\r\n");
                DEBUG_PUTS(buf);
#endif
            }
            parser_flags.measure_light=0;
        }
#endif //LIGHT_SENSOR
#ifdef DHT_SENSOR
        else if( parser_flags.measure_dht_temperature){
            int bm = parser_flags.measure_dht_temperature; // bitmask
            int sensor = (bm & 1 ? 1 : (bm & 2 ? 2 : (bm & 4 ? 3 : 4))); // which sensor?
            if (!sensor_activated[sensor + 1]) RADIO_PUTS_P(COMMAND_ERROR); // indexes 2-5
            else {
                dht_read_data( &d, &dht_temp, &dht_hum);
                sprintf_P(buf, PSTR("T%d=%0.2f\r\n"), sensor - 1, dht_temp);
#ifdef DEBUG
                DEBUG_PUTS_P("MAIN: measure_dht_temperature\r\n");
                DEBUG_PUTS(buf);
#endif
            }
            parser_flags.measure_dht_temperature=0;
        }
        else if( parser_flags.measure_dht_humidity ){
            int bm = parser_flags.measure_dht_humidity; // bitmask
            int sensor = (bm & 1 ? 1 : (bm & 2 ? 2 : (bm & 4 ? 3 : 4))); // which sensor?
            if (!sensor_activated[sensor + 1]) RADIO_PUTS_P(COMMAND_ERROR); // indexes 2-5
            else {
                dht_read_data( &d, &dht_temp, &dht_hum);
                sprintf_P(buf, PSTR("H%d=%0.2f\r\n"), sensor - 1, dht_hum);
#ifdef DEBUG
                DEBUG_PUTS_P("MAIN: measure_dht_humidity\r\n");
                DEBUG_PUTS(buf);
#endif
            }
            parser_flags.measure_dht_humidity=0;
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
