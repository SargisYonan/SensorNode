#include <avr/io.h>
#include <avr/pgmspace.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#if (__GNUC__ * 100 + __GNUC_MINOR__) < 304
#error "This library requires AVR-GCC 3.4 or later, update to newer AVR-GCC compiler !"
#endif

/*
 * constants and macros
 */

/* Enable USART 1, 2, 3 as required */
#define USART0_ENABLED
//#define USART1_ENABLED
//#define USART2_ENABLED 
//#define USART3_ENABLED

#define UART_RX0_BUFFER_SIZE 256
#define UART_RX0_BUFFER_SIZE 256
#define UART_TX1_BUFFER_SIZE 256
#define UART_TX1_BUFFER_SIZE 256
/* Set size of receive and transmit buffers */

#ifndef UART_RX0_BUFFER_SIZE
	#define UART_RX0_BUFFER_SIZE 128 /**< Size of the circular receive buffer, must be power of 2 */
#endif
#ifndef UART_RX1_BUFFER_SIZE
	#define UART_RX1_BUFFER_SIZE 128 /**< Size of the circular receive buffer, must be power of 2 */
#endif
#ifndef UART_RX2_BUFFER_SIZE
	#define UART_RX2_BUFFER_SIZE 128 /**< Size of the circular receive buffer, must be power of 2 */
#endif
#ifndef UART_RX3_BUFFER_SIZE
	#define UART_RX3_BUFFER_SIZE 128 /**< Size of the circular receive buffer, must be power of 2 */
#endif

#ifndef UART_TX0_BUFFER_SIZE
	#define UART_TX0_BUFFER_SIZE 128 /**< Size of the circular transmit buffer, must be power of 2 */
#endif
#ifndef UART_TX1_BUFFER_SIZE
	#define UART_TX1_BUFFER_SIZE 128 /**< Size of the circular transmit buffer, must be power of 2 */
#endif
#ifndef UART_TX2_BUFFER_SIZE
	#define UART_TX2_BUFFER_SIZE 128 /**< Size of the circular transmit buffer, must be power of 2 */
#endif
#ifndef UART_TX3_BUFFER_SIZE
	#define UART_TX3_BUFFER_SIZE 128 /**< Size of the circular transmit buffer, must be power of 2 */
#endif

/* Check buffer sizes are not too large for 8-bit positioning */

#if (UART_RX0_BUFFER_SIZE > 256 & !defined(USART0_LARGE_BUFFER))
	#error "Buffer too large, please use -DUSART0_LARGE_BUFFER switch in compiler options"
#endif

#if (UART_RX1_BUFFER_SIZE > 256 & !defined(USART1_LARGE_BUFFER))
	#error "Buffer too large, please use -DUSART1_LARGE_BUFFER switch in compiler options"
#endif

#if (UART_RX2_BUFFER_SIZE > 256 & !defined(USART2_LARGE_BUFFER))
	#error "Buffer too large, please use -DUSART2_LARGE_BUFFER switch in compiler options"
#endif

#if (UART_RX3_BUFFER_SIZE > 256 & !defined(USART3_LARGE_BUFFER))
	#error "Buffer too large, please use -DUSART3_LARGE_BUFFER switch in compiler options"
#endif

/* Check buffer sizes are not too large for *_LARGE_BUFFER operation (16-bit positioning) */

#if (UART_RX0_BUFFER_SIZE > 65536)
	#error "Buffer too large, maximum allowed is 65536 bytes"
#endif

#if (UART_RX1_BUFFER_SIZE > 65536)
	#error "Buffer too large, maximum allowed is 65536 bytes"
#endif

#if (UART_RX2_BUFFER_SIZE > 65536)
	#error "Buffer too large, maximum allowed is 65536 bytes"
#endif

#if (UART_RX3_BUFFER_SIZE > 65536)
	#error "Buffer too large, maximum allowed is 65536 bytes"
#endif

/** @brief  UART Baudrate Expression
 *  @param  xtalCpu  system clock in Mhz, e.g. 4000000L for 4Mhz          
 *  @param  baudRate baudrate in bps, e.g. 1200, 2400, 9600     
 */
#define UART_BAUD_SELECT(baudRate,xtalCpu) (((xtalCpu)+8UL*(baudRate))/(16UL*(baudRate))-1UL)

/** @brief  UART Baudrate Expression for ATmega double speed mode
 *  @param  xtalCpu  system clock in Mhz, e.g. 4000000L for 4Mhz           
 *  @param  baudRate baudrate in bps, e.g. 1200, 2400, 9600     
 */
#define UART_BAUD_SELECT_DOUBLE_SPEED(baudRate,xtalCpu) ((((xtalCpu)+4UL*(baudRate))/(8UL*(baudRate))-1)|0x8000)

/* test if the size of the circular buffers fits into SRAM */

#if defined(USART0_ENABLED) && ( (UART_RX0_BUFFER_SIZE+UART_TX0_BUFFER_SIZE) >= (RAMEND-0x60 ) )
#error "size of UART_RX0_BUFFER_SIZE + UART_TX0_BUFFER_SIZE larger than size of SRAM"
#endif

#if defined(USART1_ENABLED) && ( (UART_RX1_BUFFER_SIZE+UART_TX1_BUFFER_SIZE) >= (RAMEND-0x60 ) )
#error "size of UART_RX1_BUFFER_SIZE + UART_TX1_BUFFER_SIZE larger than size of SRAM"
#endif

#if defined(USART2_ENABLED) && ( (UART_RX2_BUFFER_SIZE+UART_RX2_BUFFER_SIZE) >= (RAMEND-0x60 ) )
#error "size of UART_RX2_BUFFER_SIZE + UART_TX2_BUFFER_SIZE larger than size of SRAM"
#endif

#if defined(USART3_ENABLED) && ( (UART_RX3_BUFFER_SIZE+UART_RX3_BUFFER_SIZE) >= (RAMEND-0x60 ) )
#error "size of UART_RX3_BUFFER_SIZE + UART_TX3_BUFFER_SIZE larger than size of SRAM"
#endif

/* 
** high byte error return code of uart_getc()
*/
#define UART_FRAME_ERROR      0x0800              /**< Framing Error by UART       */
#define UART_OVERRUN_ERROR    0x0400              /**< Overrun condition by UART   */
#define UART_BUFFER_OVERFLOW  0x0200              /**< receive ringbuffer overflow */
#define UART_NO_DATA          0x0100              /**< no receive data available   */

/* Macros, to allow use of legacy names */

#define uart_init(b)      uart0_init(b)
#define uart_getc()       uart0_getc()
#define uart_putc(d)      uart0_putc(d)
#define uart_puts(s)      uart0_puts(s)
#define uart_puts_p(s)    uart0_puts_p(s)
#define uart_available()  uart0_available()
#define uart_flush()      uart0_flush()

#define NULL_CHAR '\0'
#define NEWLINE_CHAR '\n'
#define BACKSPACE_CHAR 0x08

#define STRING_MULTIPLIER 2

#ifndef _UART_H_
#define _UART_H_


/*
** function prototypes
*/

/**
   @brief   Initialize UART and set baudrate 
   @param   baudrate Specify baudrate using macro UART_BAUD_SELECT()
   @return  none
*/
extern void uart0_init(uint16_t baudrate);


/**
 *  @brief   Get received byte from ringbuffer
 *
 * Returns in the lower byte the received character and in the 
 * higher byte the last receive error.
 * UART_NO_DATA is returned when no data is available.
 *
 *  @return  lower byte:  received byte from ringbuffer
 *  @return  higher byte: last receive status
 *           - \b 0 successfully received data from UART
 *           - \b UART_NO_DATA           
 *             <br>no receive data available
 *           - \b UART_BUFFER_OVERFLOW   
 *             <br>Receive ringbuffer overflow.
 *             We are not reading the receive buffer fast enough, 
 *             one or more received character have been dropped 
 *           - \b UART_OVERRUN_ERROR     
 *             <br>Overrun condition by UART.
 *             A character already present in the UART UDR register was 
 *             not read by the interrupt handler before the next character arrived,
 *             one or more received characters have been dropped.
 *           - \b UART_FRAME_ERROR       
 *             <br>Framing Error by UART
 */
extern uint16_t uart0_getc(void);

/**
 *  @brief   Peek at next byte in ringbuffer
 *
 * Returns the next byte (character) of incoming UART data without removing it from the
 * internal ring buffer. That is, successive calls to uartN_peek() will return the same
 * character, as will the next call to uartN_getc().
 *
 * UART_NO_DATA is returned when no data is available.
 *
 *  @return  lower byte:  next byte in ringbuffer
 *  @return  higher byte: last receive status
 *           - \b 0 successfully received data from UART
 *           - \b UART_NO_DATA           
 *             <br>no receive data available
 *           - \b UART_BUFFER_OVERFLOW   
 *             <br>Receive ringbuffer overflow.
 *             We are not reading the receive buffer fast enough, 
 *             one or more received character have been dropped 
 *           - \b UART_OVERRUN_ERROR     
 *             <br>Overrun condition by UART.
 *             A character already present in the UART UDR register was 
 *             not read by the interrupt handler before the next character arrived,
 *             one or more received characters have been dropped.
 *           - \b UART_FRAME_ERROR       
 *             <br>Framing Error by UART
 */
extern uint16_t uart0_peek(void);

/**
 *  @brief   Put byte to ringbuffer for transmitting via UART
 *  @param   data byte to be transmitted
 *  @return  none
 */
extern void uart0_putc(uint8_t data);


/**
 *  @brief   Put string to ringbuffer for transmitting via UART
 *
 *  The string is buffered by the uart library in a circular buffer
 *  and one character at a time is transmitted to the UART using interrupts.
 *  Blocks if it can not write the whole string into the circular buffer.
 * 
 *  @param   s string to be transmitted
 *  @return  none
 */
extern void uart0_puts(const char *s );


/**
 * @brief    Put string from program memory to ringbuffer for transmitting via UART.
 *
 * The string is buffered by the uart library in a circular buffer
 * and one character at a time is transmitted to the UART using interrupts.
 * Blocks if it can not write the whole string into the circular buffer.
 *
 * @param    s program memory string to be transmitted
 * @return   none
 * @see      uart0_puts_P
 */
extern void uart0_puts_p(const char *s );

/**
 * @brief    Macro to automatically put a string constant into program memory
 * \param    __s string in program memory
 */
#define uart_puts_P(__s)       uart0_puts_p(PSTR(__s))
#define uart0_puts_P(__s)      uart0_puts_p(PSTR(__s))

/**
 *  @brief   Return number of bytes waiting in the receive buffer
 *  @return  bytes waiting in the receive buffer
 */
extern uint16_t uart0_available(void);

/**
 *  @brief   Flush bytes waiting in receive buffer
 */
extern void uart0_flush(void);


/** @brief  Initialize USART1 (only available on selected ATmegas) @see uart_init */
extern void uart1_init(uint16_t baudrate);
/** @brief  Get received byte of USART1 from ringbuffer. (only available on selected ATmega) @see uart_getc */
extern uint16_t uart1_getc(void);
/** @brief  Peek at next byte in USART1 ringbuffer */
extern uint16_t uart1_peek(void);
/** @brief  Put byte to ringbuffer for transmitting via USART1 (only available on selected ATmega) @see uart_putc */
extern void uart1_putc(uint8_t data);
/** @brief  Put string to ringbuffer for transmitting via USART1 (only available on selected ATmega) @see uart_puts */
extern void uart1_puts(const char *s );
/** @brief  Put string from program memory to ringbuffer for transmitting via USART1 (only available on selected ATmega) @see uart_puts_p */
extern void uart1_puts_p(const char *s );
/** @brief  Macro to automatically put a string constant into program memory */
#define uart1_puts_P(__s)       uart1_puts_p(PSTR(__s))
/** @brief   Return number of bytes waiting in the receive buffer */
extern uint16_t uart1_available(void);
/** @brief   Flush bytes waiting in receive buffer */
extern void uart1_flush(void);


/** @brief  Initialize USART2 (only available on selected ATmegas) @see uart_init */
extern void uart2_init(uint16_t baudrate);
/** @brief  Get received byte of USART2 from ringbuffer. (only available on selected ATmega) @see uart_getc */
extern uint16_t uart2_getc(void);
/** @brief  Peek at next byte in USART2 ringbuffer */
extern uint16_t uart2_peek(void);
/** @brief  Put byte to ringbuffer for transmitting via USART2 (only available on selected ATmega) @see uart_putc */
extern void uart2_putc(uint8_t data);
/** @brief  Put string to ringbuffer for transmitting via USART2 (only available on selected ATmega) @see uart_puts */
extern void uart2_puts(const char *s );
/** @brief  Put string from program memory to ringbuffer for transmitting via USART2 (only available on selected ATmega) @see uart_puts_p */
extern void uart2_puts_p(const char *s );
/** @brief  Macro to automatically put a string constant into program memory */
#define uart2_puts_P(__s)       uart2_puts_p(PSTR(__s))
/** @brief   Return number of bytes waiting in the receive buffer */
extern uint16_t uart2_available(void);
/** @brief   Flush bytes waiting in receive buffer */
extern void uart2_flush(void);


/** @brief  Initialize USART3 (only available on selected ATmegas) @see uart_init */
extern void uart3_init(uint16_t baudrate);
/** @brief  Get received byte of USART3 from ringbuffer. (only available on selected ATmega) @see uart_getc */
extern uint16_t uart3_getc(void);
/** @brief  Peek at next byte in USART3 ringbuffer */
extern uint16_t uart3_peek(void);
/** @brief  Put byte to ringbuffer for transmitting via USART3 (only available on selected ATmega) @see uart_putc */
extern void uart3_putc(uint8_t data);
/** @brief  Put string to ringbuffer for transmitting via USART3 (only available on selected ATmega) @see uart_puts */
extern void uart3_puts(const char *s );
/** @brief  Put string from program memory to ringbuffer for transmitting via USART3 (only available on selected ATmega) @see uart_puts_p */
extern void uart3_puts_p(const char *s );
/** @brief  Macro to automatically put a string constant into program memory */
#define uart3_puts_P(__s)       uart3_puts_p(PSTR(__s))
/** @brief   Return number of bytes waiting in the receive buffer */
extern uint16_t uart3_available(void);
/** @brief   Flush bytes waiting in receive buffer */
extern void uart3_flush(void);





#endif // UART_H 



