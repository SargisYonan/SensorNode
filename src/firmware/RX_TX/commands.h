/*
* WRITTEN BY SARGIS S YONAN ON 22 FEB 2016
* HEADER DEFINITIONS FOR A COMMANDS LIBRARY
* USED FOR A MICRO GRID TEST BED PROJECT
* GITHUB.COM/SARGISYONAN
*/


// LIST OF ALL POSSIBLE BINARY TRANSMISSION CODES


#ifndef COMMANDS_H
#define COMMANDS_H


// SUCCESS/ERROR RETURN CODES //
#define PROCESS_COMMAND_SUCCESS                 0x01
#define ENABLE_SUCCESS            				0xDA
#define DISABLE_SUCCESS          				0xDB
#define PROCESS_COMMAND_ERROR                   0x00
#define SYSTEM_INITIALIZED                      0x11
#define INVALID_COMMAND_ERROR_CODE				0xEF

// DEBUG MODE //
#define DEBUG_ON                                  10
#define DEBUG_OFF                                  0
/////////////////////////

// RX/TX COMMANDS //

#define ENABLE 				                    0x45	//E
#define DISABLE 			                    0x44 	//D
#define GET_SENSOR_VALUE 						0x56 	//V
#define GET_SENSOR_STATUS       				0x53 	//S
#define GET_SENSOR_TYPE							0x54 	//T


#ifdef I2C_LIGHT_SENSOR
#define SEND_CURRENT_SENSOR_VALUE()				uprintf("/%d/", Sensor->currentValue)
#elif ONE_WIRE_TEMP_SENS
#define SEND_CURRENT_SENSOR_VALUE() 			uprintf("/%f/", Sensor->currentValue)
#endif
#define SEND_CURRENT_SENSOR_STATUS() 			uprintf("/%d/", Sensor->status)
#define SEND_SENSOR_TYPE()					    uprintf("/%d/", Sensor->deviceType)	


#endif