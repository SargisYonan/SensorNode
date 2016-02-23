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

#define ENABLE 				                    0xFF
#define DISABLE 			                    0xAA
#define GET_SENSOR_VALUE 						0xCC
#define GET_SENSOR_STATUS       				0x33
#define GET_SENSOR_TYPE							0x19

#define SEND_CURRENT_SENSOR_VALUE()				Sensor->currentValue[9] = '\0';\
												uprintf("/%s/", Sensor->currentValue)
#define SEND_CURRENT_SENSOR_STATUS() 			uprintf("/%d/", Sensor->status)
#define SEND_SENSOR_TYPE()					    uprintf("/%d/", Sensor->deviceType)	


#endif