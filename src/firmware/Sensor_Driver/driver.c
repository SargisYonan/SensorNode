/*
* WRITTEN BY SARGIS S YONAN ON 21 OCTOBER 2015
* HEADER DEFINITIONS FOR A SENSOR DRIVER
* USED FOR A MICRO GRID TEST BED
* GITHUB.COM/SARGISYONAN -- SARGISY@GMAIL.COM
*/


#include "driver.h"
#include "SensorStructure.h"
#include "../UART_LIBRARY/uart.h"
#include "../sensor_lib/sensor_settings.h"
#include "../RX_TX/commands.h"
#ifdef I2C_LIGHT_SENSOR
#include "../I2C_lib_new/I2C_lib.h"
#elif ONE_WIRE_TEMP_SENS
#include "../OneWire/OneWire.h"
#elif DHT11
#include "../DHT11/DHT.h"
#endif


//* CREATED BY SARGIS S YONAN - 12 OCT. 2015 */
//* A FUNCTION THAT MIMICS <stdio.h>'s PRINTF FUNCTION */
//* FOR THE AVR MICROCONTROLLER */

void uprintf (char* input_string, ...)
{
    va_list valist;
    char* newString;
    uint8_t stringLength = 0;
    
    va_start(valist, input_string);
    
    for (stringLength = 0; input_string[stringLength] != '\0'; ++stringLength) {}
    
    newString = (char*)malloc(stringLength *  STRING_MULTIPLIER);
    vsprintf(newString, input_string, valist);
    
    // WRITING TO UART STREAM //
    
    RX_TX_FUNCTION_puts(newString);
    free(newString);
    va_end(valist);
}


bool SystemInit(void)
{
	#ifdef I2C_LIGHT_SENSOR
	uint8_t txData;
	txData = 0x10;
	#endif
    sei();				/* enables AVR interrupts */
   	RX_TX_FUNCTION_init(UART_BAUD_SELECT(BAUDRATE,F_CPU));
   	
   	Sensor = (Sensor_t*)malloc(sizeof(struct SensorStructure));
   	ENABLED = false;
   	Sensor = NULL;
   	Sensor->deviceType = DEVICE_TYPE;
   	Sensor->status = 0x00;
   	#ifdef I2C_LIGHT_SENSOR
	uint16_t lightread = I2CReadValue();
	Sensor->currentValue[0] = (lightread & 0xFF00) >> 8;
	Sensor->currentValue[1] = lightread & 0xFF;
	#elif ONE_WIRE_TEMP_SENS
   	Sensor->currentValue = 0.0;
   	#elif DHT11
	initDHT();
   	Sensor->currentValue[0] = 0;
   	Sensor->currentValue[1] = 0;
   	#endif

   	uprintf("/%s/","INIT");
   	return true;
}




// the whole received message is checksummed and stored as a
// 16-bit word before the null term uint16_t with the _array_checksum algorithm
// the first 16-bits are the command, and the next bytes are the argument to that command
/*
/ example of packet to send
/  Rx    = | 0xXX 8-BIT COMMAND | 8-BIT UPPER ARGUMENT | 8-BIT DELIMITER ('-') = 0x002D |
/  index =                0                                 1                              2                               3
/ THE COMMAND -> Rx = {{RECEIVE_MESSAGE_CHANGE_SETPOINT}, {0x2D}} = changes the set point to 100 degrees Celsius
*/

void ProcessCommand(void)
{
	uint8_t rxByteArray[MAX_RECEIVE_LENGTH];
	
	for(int i = 0; i < MAX_RECEIVE_LENGTH; i++)
	{
		if (RX_TX_FUNCTION_available() < 1)
		{
			_delay_ms(XBEE_CHAR_MS_TIMEOUT);
		}
		rxByteArray[i] = RX_TX_FUNCTION_getc();
		if (rxByteArray[i] == RX_DELIMITER) 
		{
			rxByteArray[i] = '\0';
			break;
		}
	}

	switch (rxByteArray[0])
	{
		case ENABLE:
			ENABLED = true;
			uprintf("%d", ENABLE_SUCCESS);
			break;

		case DISABLE:
			ENABLED = false;
			uprintf("%d", DISABLE_SUCCESS);
			break;

		case GET_SENSOR_STATUS:
			SEND_CURRENT_SENSOR_STATUS();
			break;

		case GET_SENSOR_VALUE:
			SEND_CURRENT_SENSOR_VALUE();
			break;

		case GET_SENSOR_TYPE:
			SEND_SENSOR_TYPE();
			break;

		default:
			uprintf("%d", INVALID_COMMAND_ERROR_CODE);
			break;
	}

	RX_TX_FUNCTION_flush();
}
