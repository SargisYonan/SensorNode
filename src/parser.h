#ifndef _PARSER_H_
#define _PARSER_H_

#define MAX_SENSOR_COUNT 10
#define TOTAL_SENSOR_COUNT LIGHT_SENSOR_COUNT + DHT_SENSOR_COUNT + TEMP_SENSOR_COUNT

#define MAX_ACTUATOR_COUNT 8

/*
 * For both commands you must specify the sensor you are referring to
 */
void process_uart(void);
void parse_command(void);

/* measure_light[1:0] = Light sensors (Max 2 on one board)
 * measure_dht_*[3:0] = Humidity Sensors
 * measure_temperature[3:0] = Temperature Sensors
 * actuator_*[7:0] =  Actuators
 */
struct flags{
    uint8_t uart_error:1;
    uint8_t command_error:1;
    uint8_t command_recieved:1;
    uint8_t command_error_syntax:1;
    uint16_t value_buffer; // contains a number that was read while parsing the current command
    uint8_t set_actuator_choosesensor; // 8 bits for 8 actuators
    uint8_t get_actuator_choosesensor; // 8 bits for 8 actuators
    uint8_t set_actuator_setpoint; // 8 bits for 8 actuators
    uint8_t get_actuator_setpoint; // 8 bits for 8 actuators
    uint8_t set_actuator_onoff; // 8 bits for 8 actuators
    uint8_t get_actuator_onoff; // 8 bits for 8 actuators
    uint8_t set_actuator_armdisarm; // 8 bits for 8 actuators
    uint8_t get_actuator_armdisarm; // 8 bits for 8 actuators
    uint8_t get_info:1;
    uint8_t measure_all:1;
#ifdef DHT_SENSOR
    uint8_t measure_dht_temperature:4;
    uint8_t measure_dht_humidity:4;
#endif /* DHT */
#ifdef TEMP_SENSOR
    uint8_t measure_temperature:4;
#endif /* TEMP */
#ifdef LIGHT_SENSOR
    uint8_t measure_light:2;
#endif /* LIGHT */
} parser_flags;

#endif /* _PARSER_H_ */
