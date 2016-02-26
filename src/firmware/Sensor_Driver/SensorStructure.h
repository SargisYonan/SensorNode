// SensorStructure.h


#ifndef SENSOR_STRUCTURE_H
#define SENSOR_STRUCTURE_H


struct SensorStructure
{
	#ifdef I2C_LIGHT_SENSOR
	uint8_t currentValue[2];	// measurement
	#elif ONE_WIRE_TEMP_SENS
	float currentValue;
	#elif DHT11
	uint8_t currentValue[4];	// measurement
	#endif
	uint8_t status;		// On/Off
	uint8_t deviceType;
};
typedef struct SensorStructure Sensor_t;

#endif