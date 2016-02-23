/*
* WRITTEN BY SARGIS S YONAN ON 22 FEB 2016
* HEADER DECLARATIONS FOR A SENSOR DRIVER
* USED FOR A MICRO GRID TEST BED PROJECT
* GITHUB.COM/SARGISYONAN
*/

#ifndef DRIVER_H
#define DRIVER_H


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "../RX_TX/commands.h"
#include "../RX_TX/xbee_lib.h"
#include "SensorStructure.h"



bool ENABLED;
Sensor_t *Sensor;

//* CREATED BY SARGIS S YONAN - 12 OCT. 2015 */
//* A FUNCTION THAT MIMICS <stdio.h>'s PRINTF FUNCTION */
//* FOR THE AVR MICROCONTROLLER */

void uprintf (char* input_string, ...);


bool SystemInit(void);

void ProcessCommand(void);

/** 
 * ==============
 * NAME        : ProcessCommand -- Processes RX & TX Commands 
 * ------------
 * DESCRIPTION : This function allows the system status (set point, offsets, and state) to be configured
 *               via a list of possible commands (above). This function requires an array of the form:
                 the whole received message is checksummed and stored as a
                 16-bit word before the null term uint16_t with the _array_checksum algorithm
                 the first 16-bits are the command, and the next bytes are the argument to that command
*
/ example of packet to send
/  Rx    = | 0xXX 8-BIT COMMAND | 8-BIT UPPER ARGUMENT | 8-BIT DELIMITER ('-') = 0x002D |
/  index =                0                                 1                              2                               3
/ THE COMMAND -> Rx = {{RECEIVE_MESSAGE_CHANGE_SETPOINT}, {0x2D}} = changes the set point to 100 degrees Celsius
*
* ------------
 * PARAMETERS  : void
 * ------------
 * RETURNS     : void
 * ==============
 */

#endif