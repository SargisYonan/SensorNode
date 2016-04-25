/*
 * I2C_lib.c
 * Created by Isaak Cherdak
 * <License goes here>
 */

#include "I2C_lib.h"

/*
 * Handles all details of reading the sensor value
 */
uint16_t I2CReadValue() {
	uint16_t val;
	i2c_start(I2CWRITE); // start with write message
	i2c_write(SENSOR_COMMAND); // tell sensor we want data
	i2c_stop();
	_delay_ms(SENSOR_DELAY); // wait required time
	i2c_start(I2CREAD); // tell sensor we are going to read the data
	// first byte is the Ack, last byte is the Nak
	val = (uint8_t) i2c_readAck();
	val <<= 8;
	val |= (uint8_t) i2c_readNak();
	i2c_stop();
	// divide by 1.2 to get correctly converted reading
	return (uint16_t) val / 1.2;
}
