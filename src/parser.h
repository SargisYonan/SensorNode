#ifndef _PARSER_H_
#define _PARSER_H_

#define MAX_SENSOR_COUNT 10

/*
 * For both commands you must specify the sensor you are referring to
 */
void process_uart(int sensor);
void parse_command(int sensor);

/* parser_flags[1:0] = Light sensors (Max 2 on one board)
 * parser_flags[5:2] = Humidity Sensors
 * parser_flags[9:6] = Temperature Sensors
 */
struct flags{
    uint8_t uart_error:1;
    uint8_t command_error:1;
    uint8_t command_recieved:1;
    uint8_t command_error_setpoint:1;
    uint8_t set_setpoint:1;
    uint8_t get_setpoint:1;
    uint16_t var_setpoint;
#ifdef DHT_SENSOR
    uint8_t measure_temperature:1;
    uint8_t measure_humidity:1;
#endif /* DHT */
#ifndef DHT_SENSOR
#ifdef TEMP_SENSOR
    uint8_t measure_temperature:1;
#endif /* TEMP */
#endif /* !DHT */
#ifdef LIGHT_SENSOR
    uint8_t measure_light:1;
#endif /* LIGHT */
} parser_flags[MAX_SENSOR_COUNT];

#endif /* _PARSER_H_ */
