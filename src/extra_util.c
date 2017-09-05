#include "extra_util.h"

#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "uart.h"

#define CYCLESPERSECOND F_CPU
#define CYCLESPERMILLI CYCLESPERSECOND / 1000

static uint32_t timer1_millis;

ISR(TIMER1_COMPA_vect) {
  // increment counter or reset if hits 32 seconds
  timer1_millis = (timer1_millis >= 32000 ? 0 : timer1_millis + 1);
  TCNT1 = 0; // reset counter
}

// FIXME: the hardcode though
void timerInit(int select) {
  unsigned char sreg;
  if (select) {} // will be used eventually
  /* Save global interrupt flag */
  sreg = SREG;
  /* Disable interrupts */
  cli();

  TCCR1B |= _BV(CS10); // no prescaler
  OCR1A = CYCLESPERMILLI;
  timer1_millis = 0;
  TIMSK1 |= _BV(OCIE1A);
  TIFR1 |= _BV(OCF1A);

  /* Restore global interrupt flag */
  SREG = sreg;
  sei();

}

void timer1Reset() {
  timerSetCounter(1, 0);
  timer1_millis = 0;
}

uint32_t timer1Millis() {
  //uart_printf("\nmillis should be: %d\n", timer1_millis);
  return timer1_millis;
}

void timerSetCounter(int select, int val) {
  unsigned char sreg;
  if (select) {} // will be used eventually
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
  if (select) {} // will be used eventually
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
