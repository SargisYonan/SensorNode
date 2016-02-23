/*
* WRITTEN BY SARGIS S YONAN ON 24 NOVEMBER 2015
* HEADER DECLARATIONS FOR AN XBEE RADIO
* USED FOR A MICRO GRID TEST BED PROJECT
* GITHUB.COM/SARGISYONAN
*/

// XBEE SPECIFIC DELIMITER, DELAY, BAUDRATE, FUNCTIONS, LENGTHS
#ifndef XBEE_LIB_H
#define XBEE_LIB_H

#define RX_DELIMITER 							0x2D
#define XBEE_CHAR_MS_TIMEOUT					5
#define RX_TX_FUNCTION_getc 					uart0_getc
#define RX_TX_FUNCTION_init						uart0_init
#define RX_TX_FUNCTION_puts						uart0_puts
#define RX_TX_FUNCTION_flush					uart0_flush
#define RX_TX_FUNCTION_peek						uart0_peek
#define RX_TX_FUNCTION_available				uart0_available
#define BAUDRATE								19200

#define EIGHT_BIT_OFFSET						0x00FF

#define MAX_RECEIVE_LENGTH						3
#define MIN_RECEIVE_LENGTH						2


#endif