/*
 * =====================================================================================
 *
 *       Filename:  I2C_test.c
 *
 *    Description:  i2c test program for the GY-30 light sensor
 *
 *        Version:  0.1
 *        Created:  03/11/2016 03:52:13 PM
 *       Revision:  none
 *       Compiler:  avr-gcc
 *
 *         Author:  Zachary Graham (zwgraham@soe.ucsc.edu)
 *   Organization:  University of California, Santa Cruz
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#define BAUD 19200
#include <util/setbaud.h>
#include "../i2cmaster.h"


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  uart_putchar
 *  Description:  this simple fuction is used to define output behavior for stdio
 *                functions
 * =====================================================================================
 */
static int
uart_putchar ( char c, FILE *stream )
{
    
    if ( c == '\n' ) {
        uart_putchar( '\r', stream);
    }
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    return 0;
}		/* -----  end of function uart_putchar  ----- */


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


/*-----------------------------------------------------------------------------
 *  Global Variables
 *-----------------------------------------------------------------------------*/
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar,
                                         NULL,
                                         _FDEV_SETUP_WRITE);
/* stdout is pointed to mystdout for printf usage */
const char fmt_string[] PROGMEM = "%d lux\n";
/* prorgam memory string for printing sensor values */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  this program will continuously poll the sensor and return the
 *                value out on the uart
 * =====================================================================================
 */
int
main ( void )
{
    uint8_t i;
    uint8_t buf[2];
    uint16_t buf2;
    stdout = &mystdout;    /* link custom stdout to STDOUT */
    UBRR0H = UBRRH_VALUE;  /* configure uart baud rate */
    UBRR0L = UBRRL_VALUE;
    UCSR0B = _BV(TXEN0);   /* enable uart transmit pin */
    i2c_init();            /* initialize twi hardware */
    LEDDDR |= _BV(LED);
    for(;;){
        /* read sensor */
        i2c_start(I2CWRITE);
        i2c_write(SENSOR_COMMAND);
        i2c_stop();
        _delay_ms(SENSOR_DELAY);
        i2c_start(I2CREAD);
        buf[0] = i2c_readAck();
        buf[1] = i2c_readNak();
        i2c_stop();
        buf2 = buf[0]<<8;
        buf2 += buf[1];
//        printf_P(PSTR("%d %d\n"), buf[0], buf[1]);
        /* print sensor value */
        printf_P((PGM_P)fmt_string, (uint16_t)(buf2/1.2));

        /* delay 1 second */
        for(i=0; i<100; i++){
            _delay_ms(10);
        }
        LEDPORT ^= _BV(LED);
    }/*loop forever*/
}				/* ----------  end of function main  ---------- */
