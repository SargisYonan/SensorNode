#ifndef _UART_MACROS_H_
#define _UART_MACROS_H_

/*
 * UART Configuration Macros and Shortcuts
 * The Following macros are for UART configuration
 * and access shortcuts
 */

/**************************
 * Configuration Macros   *
 **************************/
#define RADIO_BAUD 19200
#define DEBUG_BAUD 19200
#define DEBUG_UART   uart
#define RADIO_UART   uart1

#ifdef NORADIO
#undef DEBUG
#undef RADIO_UART
#define RADIO_UART uart
#endif // NORADIO

/*
 * This next block of macros are to aid in making the UART
 * access shortcut macros.
 */
#define xCONCAT(a, b) a##b
#define CONCAT(a, b) xCONCAT(a, b)
#define INIT   _init
#define PUTS   _puts
#define PUTS_p _puts_p
#define PUTS_P _puts_P
#define GETC   _getc

/**************************
 * UART ACCCESS SHORTCUTS *
 **************************/
/* Initialize named UART
 * X_INIT(UBRRVALUES)
 * the easiest way to run is with the uart libary
 * macros:
 *     UART_BAUD_SELECT(baudRate, xtalCpu)
 *  or
 *     UART_BAUD_SELECT_DOUBLE_SPEED(baudRate, xtalCpu)
 */
#define RADIO_INIT   CONCAT(RADIO_UART, INIT)
#define DEBUG_INIT   CONCAT(DEBUG_UART, INIT)
/* Write string to named UART
 * X_PUTS(char *string)
 * X_PUTS_p(const char *progmem_string)
 * X_PUTS_P(const char *string) -> X_PUTS_p(PSTR(*string))
 * see also: uart.h
 */
#define RADIO_PUTS   CONCAT(RADIO_UART, PUTS)
#define RADIO_PUTS_P CONCAT(RADIO_UART, PUTS_P)
#define DEBUG_PUTS   CONCAT(DEBUG_UART, PUTS)
#define DEBUG_PUTS_P CONCAT(DEBUG_UART, PUTS_P)

#define RADIO_GETC   CONCAT(RADIO_UART, GETC)

#endif /* _UART_MACROS_H_ */
