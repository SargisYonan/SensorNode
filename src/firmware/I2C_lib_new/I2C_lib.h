/*
 * I2C_lib.h
 * Created by Isaak Cherdak
 * <License goes here>
 */

#ifndef _I2C_LIB_H_
#define _I2C_LIB_H_

#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#define BAUD 19200
#include <util/setbaud.h>
#include "i2cmaster.h"

/*-----------------------------------------------------------------------------
 *  Macros
 *-----------------------------------------------------------------------------*/
#define I2CADDR  0x23
#define I2CWRITE (I2CADDR)<<1
#define I2CREAD  (I2CADDR)<<1 | I2C_READ
#define SENSOR_COMMAND 0x10
#define SENSOR_DELAY 120
#define LED PB7
#define LEDDDR DDRB
#define LEDPORT PORTB

/*
 * Calls the i2cinit() function
 * May contain additional setup functionality in the future
 * Currently exists to keep consistent naming scheme
 */
#define I2CInit i2c_init

/*
 * Handles all details of reading the sensor value
 */
uint16_t I2CReadValue();

/*
 * Temporary function to initialize uart usage as well as I2C usage
 */
void I2CUartPrintInit();

/*
 * Same as I2CReadValue() but also prints to uart
 */
uint16_t I2CUartPrint();


/*
 * Runs the light based actuation demo
 */
void I2CDemo();

#endif //_I2C_LIB_H_
