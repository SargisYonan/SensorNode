#include "extra_util.h"

#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>

// FIXME: the hardcode though
void timerInit(int select) {
  TCCR1B |= _BV(CS12) | _BV(CS10); // prescaler of 1024
}

void timerSetCounter(int select, int val) {
  unsigned char sreg;
  unsigned int i;
  /* Save global interrupt flag */
  sreg = SREG;
  /* Disable interrupts */
  cli();
  /* Read TCNTn into i */
  TCNT1 = val; // FIXME: the hardcode is real (assumes select == 1)
  /* Restore global interrupt flag */
  SREG = sreg;
  sei();
}

unsigned int timerReadCounter(int select) {
  unsigned char sreg;
  unsigned int i;
  /* Save global interrupt flag */
  sreg = SREG;
  /* Disable interrupts */
  cli();
  /* Read TCNTn into i */
  i = TCNT1; // FIXME: the hardcode is real (assume select == 1)
  /* Restore global interrupt flag */
  SREG = sreg;
  sei();
  return i;
}
