/*
 * A DRIVER BY SARGIS S YONAN ON 22 FEB 2016
 * USED FOR THE MICRO GRID TEST BED
 * GITHUB.COM/SARGISYONAN -- SARGISY@GMAIL.COM
 */
#include "Sensor_Driver/driver.h"

#ifdef I2C_LIGHT_SENSOR
#include "I2C_lib_new/I2C_lib.h"
#define UPDATE_SENSOR_MEASUREMENT() I2CDemo()
#elif ONE_WIRE_TEMP_SENS
#include "OneWire/OneWire.h"
#define UPDATE_SENSOR_MEASUREMENT() Sensor->currentValue = getTemperatureC();
#elif DHT11
#include "DHT11/DHT.h"
#define UPDATE_SENSOR_MEASUREMENT() fetchData(Sensor->currentValue)
#else
UPDATE_SENSOR_MEASUREMENT() return 0
#endif

#include "UART_LIBRARY/uart.h"
#include "sensor_lib/sensor_settings.h"
#include "Helper_Library/helplib.h"
///////////////////////////////////////

int main(void)
{
	if (SystemInit()) // DEFINED IN driver.h
	{
		do
		{
			UPDATE_SENSOR_MEASUREMENT();
			if (RX_TX_FUNCTION_available() >= 1) ProcessCommand();            // if commands are in the receiving buffer
			if (ENABLED) SEND_CURRENT_SENSOR_VALUE();
		}
		while(true);    // embedded system; does not return from main()
	}
	return 0;
}
