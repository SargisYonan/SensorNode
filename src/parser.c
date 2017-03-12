#include "parser.h"

#include "uart.h"

// returns a Parser object with relevant information
// If there is an erroneous input, cmd will be set to the null character
Parser parse(char *input_str) {
  Parser p;
  p.cmd = input_str[0]; // command is first character
  const char delimit[4] = " \r\n";
  const char *token = strtok(input_str, delimit);
  if (!token) { // no tokens
    p.cmd = '\0';
    return p;
  }
  switch(token[0] & 0) { // make it give you a null, TODO: fix and remove
    case 'c':
      token = strtok(NULL, delimit);
      if (!token) { // no arguments
        p.cmd = '\0';
        return p;
      }
      p.ret_str = token; // ret_str is now type_str
      break;
    case 'i':
      break;
    case 'r':
      break;
    case 'w':
      break;
    case 'd':
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
