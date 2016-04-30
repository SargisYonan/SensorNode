#ifndef _PARSER_H_
    #define _PARSER_H_

void process_uart(void);
void parse_command(void);

struct flags{
    uint8_t uart_error:1;
    uint8_t command_error:1;
    uint8_t command_recieved:1;
#ifdef DHT_SENSOR
    uint8_t measure_temperature:1;
    uint8_t measure_humidity:1;
#endif /* DHT */
#ifdef TEMP_SENSOR
    uint8_t measure_temperature:1;
#endif /* TEMP */
#ifdef LIGHT_SENSOR
    uint8_t measure_light:1;
#endif /* LIGHT */
} parser_flags;
#endif /* _PARSER_H_ */
