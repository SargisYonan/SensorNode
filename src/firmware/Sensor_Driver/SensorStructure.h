// SensorStructure.h


#ifndef SENSOR_STRUCTURE_H
#define SENSOR_STRUCTURE_H


struct SensorStructure
{
	#ifdef I2C_LIGHT_SENSOR
	int currentValue;	// measurement
	#elif ONE_WIRE_TEMP_SENS
	float currentValue;
	#endif
	uint8_t status;		// On/Off
	uint8_t deviceType;
};
typedef struct SensorStructure Sensor_t;

#endif