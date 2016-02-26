/*
* A DRIVER BY SARGIS S YONAN ON 22 FEB 2016
* USED FOR THE MICRO GRID TEST BED
* GITHUB.COM/SARGISYONAN -- SARGISY@GMAIL.COM
*/
#include "Sensor_Driver/driver.h"

#ifdef I2C_LIGHT_SENSOR
#include "I2C_lib/i2c_master.h"
#define UPDATE_SENSOR_MEASUREMENT() i2c_receive(I2C_LOW_ADDRESS, &(Sensor->currentValue), 1)

#elif ONE_WIRE_TEMP_SENS
#include "OneWire/OneWire.h"
#define UPDATE_SENSOR_MEASUREMENT() Sensor->currentValue = getTemperatureC();
#else
UPDATE_SENSOR_MEASUREMENT() return 0
#endif

#include "UART_LIBRARY/uart.h"
#include "sensor_lib/sensor_settings.h"

///////////////////////////////////////

int main(void)
{
	if (SystemInit())	// DEFINED IN driver.h
	{
		do 
		{	
			UPDATE_SENSOR_MEASUREMENT();
			if (RX_TX_FUNCTION_available() >= 1) ProcessCommand();						// if commands are in the receiving buffer
			if (ENABLED) SEND_CURRENT_SENSOR_VALUE();
		} 
		while(true);		// embedded system; does not return from main()
	}
	return 0;
}

