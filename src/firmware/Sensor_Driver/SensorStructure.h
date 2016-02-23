// SensorStructure.h


#ifndef SENSOR_STRUCTURE_H
#define SENSOR_STRUCTURE_H

struct SensorStructure
{
	uint8_t deviceType;
	uint8_t currentValue[10];	// measurement
	uint8_t status;		// On/Off
};
typedef struct SensorStructure Sensor_t;

#endif