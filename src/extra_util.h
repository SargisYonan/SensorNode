#ifndef EXTRA_UTIL_H
#define EXTRA_UTIL_H

void timerInit(int select);

void timerSetCounter(int select, int val);

unsigned int timerReadCounter(int select);

#define timer1_init() timerInit(1)

#define timer1_setCounter(VAL) timerSetCounter(1, VAL)

#define timer1_readCounter() timerReadCounter(1)

#endif // EXTRA_UTIL_H
