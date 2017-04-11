#include "parser.h"
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "uart.h"

// sets *add_ind to the address index and *reg_bit to the corresponding values
// based off the token
// When calling this you need to give it the address of locations of the two
// uint8_t's that you would like the function to modify
// returns 1 if token is valid and numbers correctly set; 0 otherwise
uint8_t parse_pin(const char *token, uint8_t *add_ind,
    uint8_t *reg_bit, uint8_t pin_count) {
  if (token == NULL || add_ind == NULL || reg_bit == NULL) return 0;
  if (strlen((char *) token) != 3 || token[0] != 'P') { // needs to be "PXN"
    return 0;
  }
  if (token[1] < 'A' || token[1] > 'H' ||
      token[2] < '0' || token[2] > '7') { // not following "PXN"
    return 0;
  }
  add_ind[pin_count] = (token[1] - 'A');// 'A'-'H' : 0-7
  reg_bit[pin_count] = (token[2] - '0');// '0'-'7' : 0-7
  return 1;
}

// returns a Parser object with relevant information
// If there is an erroneous input, cmd will be set to the null character
Parser parse_cmd(char *input_str) {
  Parser p;
  p.cmd = input_str[0]; // command is first character
  p.ret_str = "";
  const char delimit[4] = " \r\n";
  const char *token = strtok(input_str, delimit);
  uint8_t device_index = -1;
  if (!token) { // no tokens
    p.cmd = '\0';
    return p;
  }
  switch(token[0]) {
    case CHAR_CREATE:
      {
        uint8_t add_ind[8];
        uint8_t reg_bit[8];
        p.address_index = (const uint8_t *) add_ind;
        p.reg_bit = (const uint8_t *) reg_bit;
        p.pin_count = 0;
        token = strtok(NULL, delimit);
        if (!token) { // no arguments
          p.cmd = '\0';
          return p;
        }
        p.ret_str = token; // ret_str is now type_str
        token = strtok(NULL, delimit);
        if (token == NULL) { // no mention of onboard pin location
          p.cmd = '\0';
          return p;
        }
        if (!parse_pin(token, add_ind,
              reg_bit, p.pin_count)) {
          p.cmd = '\0';
          return p;
        }
        p.pin_count++;
        while (((token = strtok(NULL, delimit)) != NULL)) {
          if (p.pin_count == 8) { //don't try to have more than 8 pins
            p.cmd = '\0';
            break;
          }
          if (!parse_pin(token, add_ind,
                reg_bit, p.pin_count)) break;
          p.pin_count++;
        }
      }
      break;
    case CHAR_INIT:
      if (!isdigit(token[1])) { // this isn't a number (but it should be)
        p.cmd = '\0';
        return p;
      }
      device_index = (uint8_t) atoi(token + 1); // index is second character
      p.device_index = device_index;
      break;
    case CHAR_READ:
      if (!isdigit(token[1])) { // this isn't a number (but it should be)
        p.cmd = '\0';
        return p;
      }
      device_index = (uint8_t) atoi(token + 1); // index is second character
      p.device_index = device_index;
      break;
    case CHAR_WRITE:
      if (!isdigit(token[1])) { // this isn't a number (but it should be)
        p.cmd = '\0';
        return p;
      }
      device_index = (uint8_t) atoi(token + 1); // index is second character
      p.device_index = device_index;
      token = strtok(NULL, delimit);
      if (!token) { // no string to write
        p.cmd = '\0';
        return p;
      }
      p.ret_str = token; // ret_str is string to write
      break;
    case CHAR_DESTROY:
      if (!isdigit(token[1])) { // this isn't a number (but it should be)
        p.cmd = '\0';
        return p;
      }
      device_index = (uint8_t) atoi(token + 1); // index is second character
      p.device_index = device_index;
      break;
    case CHAR_KILL:
      if (!isdigit(token[1])) { // this isn't a number (but it should be)
        p.cmd = '\0';
        return p;
      }
      device_index = (uint8_t) atoi(token + 1); // index is second character
      p.device_index = device_index;
      break;
    case CHAR_MAP:
      // nothing to do
      break;
    default:
      p.cmd = '\0'; // invalid command
      break;
  }
  return p;
}
