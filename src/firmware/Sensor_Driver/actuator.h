#ifndef _ACTUATOR_H_
#define _ACTUATOR_H_

#include <avr/io.h>
#include <util/delay.h> 

#define ACT_PORT_OUT PORTB
#define ACT_PORT_IN PINB
#define ACT_PIN PB5
#define ACT_DDR DDRB

#define CLEAR_DDR() (ACT_DDR &= 0x00)
#define SET_BIT(byte, bit) ((byte) |= (1UL << (bit)))
#define CLEAR_BIT(byte,bit) ((byte) &= ~(1UL << (bit)))
#define IS_SET(byte,bit) (((byte) & (1UL << (bit))) >> (bit))

void TurnOffPin(void);
void TurnOnPin(void);
void PinInit(void);

#endif