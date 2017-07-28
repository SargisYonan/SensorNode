#ifndef CURRENT_SENSOR_TRANSMIT_H_
#define CURRENT_SENSOR_TRANSMIT_H_

#include "module.h"

#ifndef CURRENT_SENSOR_MAX
#define CURRENT_SENSOR_MAX 10
#endif

#define CURRENT_SENSOR_IDENTIFIER_STRING "CURRENT_SENSOR"

typedef Module Current_Sensor;

Current_Sensor new_current_sensor(uint8_t, Current_Sensor);

void current_sensor_init(Current_Sensor); // default init function
void current_sensor_read(Current_Sensor, char *, uint16_t); // read function
void current_sensor_destroy(Current_Sensor); // default destroy function

#endif
