/*
 * I2C_lib.c
 * Created by Isaak Cherdak
 * <License goes here>
 */

#include "I2C_lib.h"

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
 *  Global Variables
 *-----------------------------------------------------------------------------*/
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar,
		NULL,
		_FDEV_SETUP_WRITE);
/* stdout is pointed to mystdout for printf usage */
const char fmt_string[] PROGMEM = "%d lux\n";
/* prorgam memory string for printing sensor values */



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
	val = ((uint16_t) ((uint8_t) i2c_readAck()) << 8) + ((uint8_t) i2c_readNak());
	i2c_stop();
	// divide by 1.2 to get correctly converted reading
	return (uint16_t) (val / 1.2);
}

/*
 * DO NOT USE THE BELOW FUNCTIONS
 */

/*
 * Temporary function to initialize uart usage as well as I2C usage
 */
void I2CUartPrintInit() {
	stdout = &mystdout;    /* link custom stdout to STDOUT */
	UBRR0H = UBRRH_VALUE;  /* configure uart baud rate */
	UBRR0L = UBRRL_VALUE;
	UCSR0B = _BV(TXEN0);   /* enable uart transmit pin */
	I2CInit();
}

/*
 * Same as I2CReadValue() but also prints to uart
 */
uint16_t I2CUartPrint() {
	uint16_t temp = I2CReadValue();
	printf_P((PGM_P)fmt_string, temp);
	return temp;
}

#define LOWERBOUND 10
#define UPPERBOUND 14
void I2CDemo() {
	I2CUartPrintInit();
	LEDDDR |= _BV(LED);
	uint16_t temp;
	uint8_t flag = 0;
	uint8_t changed = 0;
	uint8_t iter;
	for (;;) {
		for (iter = 0; iter < 100; iter++) _delay_ms(10);
		temp = I2CUartPrint();
		if (temp <= LOWERBOUND && !flag) {
			flag = 1;
			changed = 1;
		} else if (temp >= UPPERBOUND && flag) {
			flag = 0;
			changed = 1;
		}
		if (changed) {
			if (flag) LEDPORT |= _BV(LED);
			else LEDPORT &= ~_BV(LED);
		}
		changed = 0;
	}
}
