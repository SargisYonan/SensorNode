#ifndef EXTRA_UTIL_H
#define EXTRA_UTIL_H

#include <stdint.h>

void timerInit(int select);

void timerSetCounter(int select, int val);

unsigned int timerReadCounter(int select);

void timer1Reset(void);

uint32_t timer1Millis(void);

#define timer1_init() timerInit(1)

#define timer1_setCounter(VAL) timerSetCounter(1, VAL)

#define timer1_readCounter() timerReadCounter(1)

#endif // EXTRA_UTIL_H
