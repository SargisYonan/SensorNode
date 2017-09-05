#ifndef HUMIDITY_SENSOR_H_
#define HUMIDITY_SENSOR_H_

#include <string.h>
#include "module.h"

#ifndef HUMIDITY_SENSOR_MAX
#define HUMIDITY_SENSOR_MAX 1
#endif

#define HUMIDITY_SENSOR_IDENTIFIER_STRING "HUMIDITY_SENSOR"

typedef Module Humidity_Sensor;

Humidity_Sensor new_humidity_sensor(uint8_t, Humidity_Sensor);

void humidity_sensor_init(Humidity_Sensor);

void humidity_sensor_read(Humidity_Sensor, char *, uint16_t);

void humidity_sensor_destroy(Humidity_Sensor);

#endif
