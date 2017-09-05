#ifndef LIGHT_SENSOR_H_
#define LIGHT_SENSOR_H_

#include <string.h>
#include "module.h"

#ifndef LIGHT_SENSOR_MAX
#define LIGHT_SENSOR_MAX 1
#endif

#define LIGHT_SENSOR_IDENTIFIER_STRING "LIGHT_SENSOR"

typedef Module Light_Sensor;

Light_Sensor new_light_sensor(uint8_t, Light_Sensor);

void light_sensor_init(Light_Sensor);

void light_sensor_read(Light_Sensor, char *, uint16_t);

void light_sensor_destroy(Light_Sensor);

#endif
