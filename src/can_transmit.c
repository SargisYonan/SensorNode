#include "can_transmit.h"

// TODO: write this

uint8_t can_transmit_count = 0;
uint8_t can_transmit_type_num = -1; // needs to be set on first creation of Can_Transmit

// sets an index of the can_transmit module array to be the new can_transmit's info
// also sets the fields accordingly
// RETURNS:
// the can_transmit with fields sest appropriately
// or a default module if too many can_transmits already exist
Can_Transmit new_can_transmit(uint8_t cur_type_num, volatile uint8_t *port,
    volatile uint8_t *pin, volatile uint8_t *ddr, uint8_t reg_bit) {
  Can_Transmit ct = new_module();
  if (can_transmit_count >= CAN_TRANSMIT_MAX) {
    return ct; // remember the key is that it has defaults set
  }
  if (can_transmit_count == 0) {
    can_transmit_type_num = cur_type_num;
  }
  ct.type_num = can_transmit_type_num;
  ct.type_string = CAN_TRANSMIT_IDENTIFIER_STRING;
  ct.index = can_transmit_count++;
  ct.port = port;
  ct.pin = pin;
  ct.ddr = ddr;
  ct.reg_bit = reg_bit;
  ct.init = &can_transmit_init;
  ct.write = &can_transmit_write;
  ct.destroy = &can_transmit_destroy;
  return ct;
}

// currently a hardcoded solution
void *can_transmit_init(Can_Transmit a) {

  // Hardcoded stuff to play with CAN trancievers

  OCR0A = 1; // one cycle for timer

  //ASSR |= _BV(OCIE0A); // set bit of async status register for following line
  TIMSK0 |= _BV(OCIE0A); // set appropriate bit of timer interrupt register to enable compareA interrupt

  TCCR0A |= _BV(COM0A0);
  TCCR0A |= _BV(WGM00) | _BV(WGM01);
  // timer prescaler to FREQ/1024 or ~16kHz increments
  TCCR0B |= _BV(CS02) | _BV(CS00) | _BV(WGM02);
  DDRB |= _BV(PB7);
  while(1) { // system loop
    if (TIFR0 & _BV(OCF0A)) { // ~16 kHz

      TIFR0 |= _BV(OCF0A); // clear timer interrupt flag (by setting)
      TCNT0 = 0; // clear value stored in timer

    }
  }


  return (void *) "";
}

void *can_transmit_write(Can_Transmit a, void *origstr) {
  return origstr;
}

void *can_transmit_destroy(Can_Transmit a) {
  return (void *) "";
}
