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
  char cmd; // character for the cmd, ie: 'c', 'h', 'd', 'i', etc
  uint8_t device_index; // index of the device array that we should access
  // the below are only relevant when being given 'c' for a cmd
  const char *type_str; // string for type ie: "ACTUATOR", etc
  uint8_t port_address_index; // also index of address for pin and ddr
  uint8_t reg_bit; // bit of the port that was requested
} Parser;

Parser parse_cmd(char *);

#endif /* _PARSER_H_ */
