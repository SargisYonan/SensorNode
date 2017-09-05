#ifndef _PARSER_H_
#define _PARSER_H_

#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>

typedef struct Parser{
  char cmd; // character for the cmd, ie: 'c', 'm', 'd', 'i', 'r', 'w', 'k'
  uint8_t device_index; // index of the device array that we should access
  // TODO: use PROGMEM for the three following fields
  const char *ret_str; // ie: cmd = 'c': type_str, cmd = 'w': write_str
  // the below are only relevant when being given 'c' for a cmd
  const uint8_t *address_index; // array of indices for addresses of
                                // port, pin, and ddr requested
  const uint8_t *reg_bit; // array of bits of the ports that were requested
  uint8_t pin_count; // number of pins assigned
  // TODO: should we have an error flag? will it be useful to have diffent kinds
  // of errors printing?
} Parser;

Parser parse_cmd(char *);

#endif /* _PARSER_H_ */
