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
  const char *ret_str; // ie: cmd = 'c': type_str, cmd = 'w': write_str
  // the below are only relevant when being given 'c' for a cmd
  uint8_t address_index; // index of address for port, pin, and ddr
  uint8_t reg_bit; // bit of the port that was requested
  // TODO: should we have an error flag? will it be useful to have diffent kinds
  // of errors printing?
} Parser;

Parser parse_cmd(char *);

#endif /* _PARSER_H_ */
