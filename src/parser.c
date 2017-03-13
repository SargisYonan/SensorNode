#include "parser.h"

#include <ctype.h>

#include "uart.h"

// returns a Parser object with relevant information
// If there is an erroneous input, cmd will be set to the null character
Parser parse(char *input_str) {
  Parser p;
  p.cmd = input_str[0]; // command is first character
  const char delimit[4] = " \r\n";
  const char *token = strtok(input_str, delimit);
  uint8_t device_index = -1;
  if (!token) { // no tokens
    p.cmd = '\0';
    return p;
  }
  switch(token[0]) {
    case 'c': // TODO: handle multiple pins for one device?
      token = strtok(NULL, delimit);
      if (!token) { // no arguments
        p.cmd = '\0';
        return p;
      }
      p.ret_str = token; // ret_str is now type_str
      token = strtok(NULL, delimit);
      if (!token) { // no mention of onboard pin location
        p.cmd = '\0';
        return p;
      }
      if (strlen((char *) token) != 3 || token[0] != 'P') { // needs to be "PXN"
        p.cmd = '\0';
        return p;
      }
      if (!isalpha(token[1] || !isdigit(token[2]))) { // not following "PXN"
        p.cmd = '\0';
        return p;
      }
      p.address_index = (uint8_t) token[1] - 'A'; // 'A'-'H' : 0-7
      p.reg_bit = (uint8_t) token[1] - '0'; // '0'-'7' : 0-7
      break;
    case 'i':
      if (!isdigit(token[1])) { // this isn't a number (but it should be)
        p.cmd = '\0';
        return p;
      }
      device_index = (uint8_t) atoi(token + 1);
      p.device_index = device_index;
      break;
    case 'r':
      if (!isdigit(token[1])) { // this isn't a number (but it should be)
        p.cmd = '\0';
        return p;
      }
      device_index = (uint8_t) atoi(token + 1);
      p.device_index = device_index;
      break;
    case 'w':
      break;
    case 'd':
      if (!isdigit(token[1])) { // this isn't a number (but it should be)
        p.cmd = '\0';
        return p;
      }
      device_index = (uint8_t) atoi(token + 1);
      p.device_index = device_index;
      break;
    case 'k':
      break;
    case 'm':
      break;
    default:
      p.cmd = '\0'; // invalid command
      break;
  }
  return p;
}
