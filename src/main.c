#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
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

// device: 0: primary, 1: debug
void S_PUTS(char *buf, int device) {
    char temp[strlen(buf + 1)]; // we don't want to modify the original array
    char temp2[11] = {}; // this is what happens when the compiler doesn't like pointer arithmetic
    strncpy(temp, buf, strlen(buf) + 1);
    //sprintf_P(temp, PSTR("buflen:%d\n"), (int) strlen(temp));
    //RADIO_PUTS(temp);
    int i;
    for (i = 0;; i += 10) { // print every 15 characters
        if (strlen(temp) - i > 10) {
            char tempc = temp[10 + i];
            temp[10 + i] = '\0';
            sprintf_P(temp2, PSTR("%s"), temp + i);
            if (device == 0) RADIO_PUTS(temp2);
            else if (device == 1) DEBUG_PUTS(temp2);
            temp2[0] = '\0';
            temp[10 + i] = tempc;
        } else {
            break;
        }
    }
    //print the remaining characters
    sprintf_P(temp2, PSTR("%s"), temp + i);
    if (device == 0) RADIO_PUTS(temp2);
    else if (device == 1) DEBUG_PUTS(temp2);
}


static void
serial_init(void){
#ifdef DEBUG
    DEBUG_INIT(UART_BAUD_SELECT(DEBUG_BAUD, F_CPU));
#endif
    RADIO_INIT(UART_BAUD_SELECT(RADIO_BAUD, F_CPU));
}

#define COMMAND_ERROR "BAD COMMAND\r\n"

#define ACTUATOR0 PB0
#define ACTUATOR1 PB1
#define ACTUATOR2 PB2
#define ACTUATOR3 PB3
#define ACTUATOR4 PB4
#define ACTUATOR5 PB5
#define ACTUATOR6 PB6
#define ACTUATOR7 PB7
#define ACTUATORDDR DDRB
#define ACTUATORPORT PORTB

int
main(void){
    char buf[1024];
    buf[0] = '\0';
    uint32_t ledcount = 0; // counter to slow down LED access
    if (ACTUATOR_COUNT > 0) ACTUATORDDR |= _BV(ACTUATOR0);
    if (ACTUATOR_COUNT > 1) ACTUATORDDR |= _BV(ACTUATOR1);
    if (ACTUATOR_COUNT > 2) ACTUATORDDR |= _BV(ACTUATOR2);
    if (ACTUATOR_COUNT > 3) ACTUATORDDR |= _BV(ACTUATOR3);
    if (ACTUATOR_COUNT > 4) ACTUATORDDR |= _BV(ACTUATOR4);
    if (ACTUATOR_COUNT > 5) ACTUATORDDR |= _BV(ACTUATOR5);
    if (ACTUATOR_COUNT > 6) ACTUATORDDR |= _BV(ACTUATOR6);
    if (ACTUATOR_COUNT > 7) ACTUATORDDR |= _BV(ACTUATOR7);
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
        sprintf(buf, "No sensors installed, operation will now halt\r\n");
        S_PUTS(buf, 0);
        for(;;);
    }
    if (LIGHT_SENSOR_COUNT > 1 || DHT_SENSOR_COUNT > 1 || TEMP_SENSOR_COUNT > 1) {
        sprintf(buf, "Cannot support more than one of a single type of sensor yet\r\n");
        S_PUTS(buf, 0);
        for(;;);
    }
    // the following 4 are for storing settings for the sensors
    uint8_t actuator_ports[MAX_ACTUATOR_COUNT] = {ACTUATOR0, ACTUATOR1, ACTUATOR2, ACTUATOR3,
        ACTUATOR4, ACTUATOR5, ACTUATOR6, ACTUATOR7};
    uint8_t actuator_onoff = 0; //bitmask it
    uint8_t actuator_armdisarm = 0; //bitmask it
    uint8_t actuator_status = 0;
    uint16_t actuator_setpoint[MAX_ACTUATOR_COUNT] = {}; // each index is a number so you can't bitmask it
    uint8_t actuator_sensor[MAX_ACTUATOR_COUNT] = {}; // each index is a number so you can't bitmask it
    uint8_t sensor_activated[MAX_SENSOR_COUNT] = {}; /*
                                                      * Tells which sensors are installed
                                                      * [1:0]: Light sensors
                                                      * [5:2]: DHT Sensors
                                                      * [9:6]: Temp Sensors
                                                      */

    for (int i = 0; i < LIGHT_SENSOR_COUNT && i < 2; i++) sensor_activated[i] = 1; // validate light sensors
    for (int i = 2; i < DHT_SENSOR_COUNT + 2 && i < 6; i++) sensor_activated[i] = 1; // validate humidity sensors
    for (int i = 6; i < TEMP_SENSOR_COUNT + 6 && i < 10; i++) sensor_activated[i] = 1; // validate temperature sensors

    int first_sensor_activated = 0; // make sure that you default to an activated sensor
    for (int i = 0; i < MAX_SENSOR_COUNT; i++) {
        if (sensor_activated[i]) {
            first_sensor_activated = i;
            break;
        }
    }
    for (int i = 0; i < MAX_ACTUATOR_COUNT; i++) {
        actuator_sensor[i] = first_sensor_activated;
    }
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
            sprintf(buf, "ND:L=%d D=%d T=%d A=%d\r\n",
                    LIGHT_SENSOR_COUNT, DHT_SENSOR_COUNT, TEMP_SENSOR_COUNT, ACTUATOR_COUNT);
            S_PUTS(buf, 0);
            buf[0] = '\0';
            parser_flags.get_info = 0;
        }
        else if (parser_flags.measure_all) {
            char strlight[64] = "";
            char strdht[64] = "";
            char strtemp[64] = "";
            char stract[128] = "";
            char str[128] = "";
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
                strcat(strdht, str);
            }
#endif // DHT_SENSOR
#ifdef TEMP_SENSOR
            for (int i = 6; i < 10 && sensor_activated[i]; i++) {
                temp = getTemperatureC();
                sprintf(str, "T%d=%0.2f ", i - 6, temp);
                strcat(strtemp, str);
            }
#endif // TEMP_SENSOR
            for (int i = 0; i < ACTUATOR_COUNT; i++) {
                sprintf(str, "AO%d=%d AA%d=%d AP%d=%d AS%d=%c%d AT%d=%d ", i, (actuator_onoff & _BV(i)) >> i,
                        i, (actuator_armdisarm & _BV(i)) >> i, i, actuator_setpoint[i], i,
                        actuator_sensor[i] < 2 ? 'L' : actuator_sensor[i] < 6 ? 'D' : 'T',
                        actuator_sensor[i] < 2 ? actuator_sensor[i] : actuator_sensor[i] < 6 ? actuator_sensor[i] - 2 : actuator_sensor[i] - 6,
                        i, (actuator_status & _BV(i)) >> i);
                strcat(stract, str);
            }
            sprintf(buf, "%s%s%s%s\r\n", strlight, strdht, strtemp, stract);
            S_PUTS(buf, 0);
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
                sprintf_P(buf, PSTR("DT%d=%0.2f\r\n"), sensor - 1, dht_temp);
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
                sprintf_P(buf, PSTR("DH%d=%0.2f\r\n"), sensor - 1, dht_hum);
#ifdef DEBUG
                DEBUG_PUTS_P("MAIN: measure_dht_humidity\r\n");
                DEBUG_PUTS(buf);
#endif
            }
            parser_flags.measure_dht_humidity=0;
        }
#endif //DHT_SENSOR
        else if (parser_flags.set_actuator_onoff) {
            if (!parser_flags.command_error_syntax) {
                int actuatorbitmask = parser_flags.set_actuator_onoff;
                int actuator = 0;
                for (int i = 0;; i++) { // un-bitmask the value
                    // (is that
                    // even a
                    // word?)
                    if (_BV(i) == actuatorbitmask) {
                        actuator = i;
                        break;
                    }
                }
                if (actuator < ACTUATOR_COUNT) {
                    // if we got a zero, make this bit 1, else make it 0
                    if (!parser_flags.value_buffer) actuator_onoff &= ~actuatorbitmask;
                    else actuator_onoff |= actuatorbitmask;
                    sprintf_P(buf, PSTR("set AO%d=%s\r\n"), actuator, parser_flags.value_buffer ? "1" : "0");
                } else {
                    RADIO_PUTS_P(COMMAND_ERROR);
                }
            }
            parser_flags.set_actuator_onoff = 0;
        }
        else if (parser_flags.get_actuator_onoff) {
            int actuatorbitmask = parser_flags.get_actuator_onoff;
            int actuator = 0;
            for (int i = 0;; i++) { // un-bitmask the value
                // (is that
                // even a
                // word?)
                if (_BV(i) == actuatorbitmask) {
                    actuator = i;
                    break;
                }
            }
            if (actuator < ACTUATOR_COUNT) {
                sprintf_P(buf, PSTR("AO%d=%s\r\n"), actuator, actuator_onoff & actuatorbitmask ? "1" : "0");
            } else {
                RADIO_PUTS_P(COMMAND_ERROR);
            }
            parser_flags.get_actuator_onoff = 0;
        }
        else if (parser_flags.set_actuator_armdisarm) {
            if (!parser_flags.command_error_syntax) {
                int actuatorbitmask = parser_flags.set_actuator_armdisarm;
                int actuator = 0;
                for (int i = 0;; i++) { // un-bitmask the value
                    // (is that
                    // even a
                    // word?)
                    if (_BV(i) == actuatorbitmask) {
                        actuator = i;
                        break;
                    }
                }
                if (actuator < ACTUATOR_COUNT) {
                    // if we got a zero, make this bit 1, else make it 0
                    if (!parser_flags.value_buffer) actuator_armdisarm &= ~actuatorbitmask;
                    else actuator_armdisarm |= actuatorbitmask;
                    sprintf_P(buf, PSTR("set AA%d=%s\r\n"), actuator, parser_flags.value_buffer ? "1" : "0");
                } else {
                    RADIO_PUTS_P(COMMAND_ERROR);
                }
            }
            parser_flags.set_actuator_armdisarm = 0;
        }
        else if (parser_flags.get_actuator_armdisarm) {
            int actuatorbitmask = parser_flags.get_actuator_armdisarm;
            int actuator = 0;
            for (int i = 0;; i++) { // un-bitmask the value
                // (is that
                // even a
                // word?)
                if (_BV(i) == actuatorbitmask) {
                    actuator = i;
                    break;
                }
            }
            if (actuator < ACTUATOR_COUNT) {
                sprintf_P(buf, PSTR("AA%d=%s\r\n"), actuator, actuator_armdisarm & actuatorbitmask ? "1" : "0");
            } else {
                RADIO_PUTS_P(COMMAND_ERROR);
            }
            parser_flags.get_actuator_armdisarm = 0;
        }
        else if (parser_flags.set_actuator_setpoint) {
            if (!parser_flags.command_error_syntax) {
                int actuatorbitmask = parser_flags.set_actuator_setpoint;
                int actuator = 0;
                for (int i = 0;; i++) { // un-bitmask the value
                    // (is that
                    // even a
                    // word?)
                    if (_BV(i) == actuatorbitmask) {
                        actuator = i;
                        break;
                    }
                }
                if (actuator < ACTUATOR_COUNT) {
                    actuator_setpoint[actuator] = parser_flags.value_buffer;
                    sprintf(buf, "set AP%d=%d\r\n", actuator, parser_flags.value_buffer);
                    S_PUTS(buf, 0);
                    buf[0] = '\0';
                } else {
                    RADIO_PUTS_P(COMMAND_ERROR);
                }
            }
            parser_flags.set_actuator_setpoint = 0;
        }
        else if (parser_flags.get_actuator_setpoint) {
            int actuatorbitmask = parser_flags.get_actuator_setpoint;
            int actuator = 0;
            for (int i = 0;; i++) { // un-bitmask the value
                // (is that
                // even a
                // word?)
                if (_BV(i) == actuatorbitmask) {
                    actuator = i;
                    break;
                }
            }
            if (actuator < ACTUATOR_COUNT) {
                sprintf_P(buf, PSTR("AP%d=%d\r\n"), actuator, actuator_setpoint[actuator]);
            } else {
                RADIO_PUTS_P(COMMAND_ERROR);
            }
            parser_flags.get_actuator_setpoint = 0;
        }
        else if (parser_flags.set_actuator_choosesensor) {
            if (!parser_flags.command_error_syntax) {
                int actuatorbitmask = parser_flags.set_actuator_choosesensor;
                int actuator = 0;
                for (int i = 0;; i++) { // un-bitmask the value
                    // (is that
                    // even a
                    // word?)
                    if (_BV(i) == actuatorbitmask) {
                        actuator = i;
                        break;
                    }
                }
                // check that the sensor we want to base off of is actually
                // installed
                if (actuator < ACTUATOR_COUNT && sensor_activated[parser_flags.value_buffer]) {
                    int tempi = parser_flags.value_buffer;
                    actuator_sensor[actuator] = tempi;
                    char tempc = tempi < 2 ? 'L' : tempi < 6 ? 'D' : 'T'; // which sensor?
                    tempi -= tempc == 'L' ? 0 : tempc == 'D' ? 2 : 6; // out of the type of sensor, which one?
                    sprintf_P(buf, PSTR("set AS%d=%c%d\r\n"), actuator, tempc, tempi);
                } else {
                    RADIO_PUTS_P(COMMAND_ERROR);
                }
            }
            parser_flags.set_actuator_choosesensor = 0;
        }
        else if (parser_flags.get_actuator_choosesensor) {
            int actuatorbitmask = parser_flags.get_actuator_choosesensor;
            int actuator = 0;
            for (int i = 0;; i++) { // un-bitmask the value
                // (is that
                // even a
                // word?)
                if (_BV(i) == actuatorbitmask) {
                    actuator = i;
                    break;
                }
            }
            if (actuator < ACTUATOR_COUNT) {
                int tempi = actuator_sensor[actuator];
                char tempc = tempi < 2 ? 'L' : tempi < 6 ? 'D' : 'T'; // which sensor?
                tempi -= tempc == 'L' ? 0 : tempc == 'D' ? 2 : 6; // out of the type of sensor, which one?
                sprintf_P(buf, PSTR("AS%d=%c%d\r\n"), actuator, tempc, tempi);
            } else {
                RADIO_PUTS_P(COMMAND_ERROR);
            }
            parser_flags.get_actuator_choosesensor = 0;
        }
        else if (parser_flags.get_actuator_status) {
            int actuatorbitmask = parser_flags.get_actuator_status;
            int actuator = 0;
            for (int i = 0;; i++) { // un-bitmask the value
                // (is that
                // even a
                // word?)
                if (_BV(i) == actuatorbitmask) {
                    actuator = i;
                    break;
                }
            }
            if (actuator < ACTUATOR_COUNT) {
                sprintf_P(buf, PSTR("AT%d=%d\r\n"), actuator, actuator_status & actuatorbitmask ? 1 : 0);
            } else {
                RADIO_PUTS_P(COMMAND_ERROR);
            }
            parser_flags.get_actuator_status = 0;
        }
        if (parser_flags.command_error_syntax) {
            sprintf_P(buf, PSTR("BAD COMMAND 1\r\n"));
#ifdef DEBUG
            DEBUG_PUTS_P("MAIN: parser_flags.command_error_syntax=1\r\n");
#endif
            parser_flags.command_error_syntax = 0;
        }
        else if(buf[0]){
            RADIO_PUTS(buf);
            buf[0]='\0';
        }
        if (ledcount-- <= 0) {
            ledcount = F_CPU / 1e3;
#ifdef LIGHT_SENSOR
            light = I2CReadValue();
#endif //LIGHT_SENSOR
#ifdef DHT_SENSOR
            dht_read_data(&d, &dht_temp, &dht_hum);
#endif //DHT_SENSOR
#ifdef TEMP_SENSOR
            temp = getTemperatureC();
#endif //TEMP_SENSOR
            for (int i = 0; i < ACTUATOR_COUNT; i++) {
                if (actuator_onoff & _BV(i)) {
                    if (actuator_armdisarm & _BV(i)) { // force on
                        ACTUATORPORT |= _BV(actuator_ports[i]);
                        actuator_status |= _BV(i);
                    } else { // setpoint based
                        if (actuator_sensor[i] < 2) { // this works off an assumption of at most one of each type of sensor. light sensor here
#ifdef LIGHT_SENSOR
                            if (light < 0.8 * actuator_setpoint[i]) {
                                ACTUATORPORT |= _BV(actuator_ports[i]);
                                actuator_status |= _BV(i);
                            } else if (light > 1.2 * actuator_setpoint[i]) {
                                ACTUATORPORT &= ~_BV(actuator_ports[i]);
                                actuator_status &= ~_BV(i);
                            }
#endif // LIGHT_SENSOR
                        } else if (actuator_sensor[i] < 6) { // dht sensor (in this case we assume you want to work off humidity)
#ifdef DHT_SENSOR
                            if (dht_hum < 0.8 * actuator_setpoint[i]) {
                                ACTUATORPORT |= _BV(actuator_ports[i]);
                                actuator_status |= _BV(i);
                            } else if (dht_hum > 1.2 * actuator_setpoint[i]) {
                                ACTUATORPORT &= ~_BV(actuator_ports[i]);
                                actuator_status &= ~_BV(i);
                            }
#endif // DHT_SENSOR
                        } else { // temp sensor, may change if more sensors are added to system
#ifdef TEMP_SENSOR
                            if (temp < 0.8 * actuator_setpoint[i]) {
                                ACTUATORPORT |= _BV(actuator_ports[i]);
                                actuator_status |= _BV(i);
                            } else if (temp > 1.2 * actuator_setpoint[i]) {
                                ACTUATORPORT &= ~_BV(actuator_ports[i]);
                                actuator_status &= ~_BV(i);
                            }
#endif // TEMP_SENSOR
                        }
                    }
                } else { // force off
                    ACTUATORPORT &= ~_BV(actuator_ports[i]);
                    actuator_status &= ~_BV(i);
                }
            }
        }
    }
}
