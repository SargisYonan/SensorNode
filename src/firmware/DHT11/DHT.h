#ifndef _DHT_h
#define _DHT_h

#define DHT_PORT_OUT PORTC
#define DHT_PORT_IN PINC
#define DHT_PIN PC0

#define SET_BIT(byte, bit) ((byte) |= (1UL << (bit)))

#define CLEAR_BIT(byte,bit) ((byte) &= ~(1UL << (bit)))

#define IS_SET(byte,bit) (((byte) & (1UL << (bit))) >> (bit))

#include <stdint.h>

void initDHT(void);

uint8_t fetchData(uint8_t* arr);

#endif