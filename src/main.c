#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>
#ifdef DHT_SENSOR
#include "dht.h"
#endif // DHT_SENSOR
#ifdef TEMP_SENSOR
#include "OneWire.h"
#endif // TEMP_SENSOR
#ifdef LIGHT_SENSOR
#include "I2C_lib.h"
#endif // LIGHT_SENSOR
#include "uart.h"
#include "module.h"
#include "actuator.h"

#define MAX_DEVICES 100

#define COMMAND_LONG (unsigned char *) \
  "COMMAND EXCEEDED TX_BUF_SIZE Characters\r\n"

typedef Module (* NEW_DEVICE_FUNC_TYPE) (uint8_t, volatile uint8_t *,
      volatile uint8_t *, volatile uint8_t *, uint8_t);

Module devices[MAX_DEVICES]; // all devices currently running
uint8_t devices_valid[MAX_DEVICES]; // device exists if its index contains 1
// type i points to a corresponding string
const char *type_num_to_string_map[MAX_DEVICES];
// type i points to a corresponding creation function
NEW_DEVICE_FUNC_TYPE type_num_to_create_function_map [MAX_DEVICES];
uint8_t num_types = 0; // track the number of different types
uint8_t device_next; // next index to create a device in
uint8_t devices_count = 0;
//TODO: this is obsolete, get it out of here
uint8_t type_cur_num = 0; // a number tracking the next device type to assign

// TODO: reolve name to ID (just do a linear search)

// called initially for every type of module
void add_to_resolvers(uint8_t type_num, char const *type_string,
    NEW_DEVICE_FUNC_TYPE new_device) {
  type_num_to_string_map[type_num] = type_string;
  type_num_to_create_function_map[type_num] =
    new_device;
}

// TODO: This should run through a parser to determine the on-board location
// based on the command
// currently hardcoded
void create_device(uint8_t device, char *cmd) {
  devices[device] = type_num_to_create_function_map[0] // type for actuator = 0
    (0, &PORTA, &PINA, &DDRA, cmd[0] == 'r' ? PA0 : cmd[0] == 'g' ? PA1 : PA2);
}

// TODO: make the parser and the above function first
void remove_device(uint8_t device) {
  // compiler annoyance
  device = 5;
  device = device + 1;
}

int main(void){

  // TODO: This part needs to be protected by a set of ifndef's per module
  add_to_resolvers(num_types++, ACTUATOR_IDENTIFIER_STRING, &new_actuator);

  sei();
  
  uart_init();

  unsigned char cmd[TX_BUF_SIZE + 1]; // max amount written by uart_ngetc()
  uint16_t cmd_index = 0;

  // temporary hardcode
  create_device(devices_count++, (char *) "r"); // start with red
  uart_puts(devices[0].init(devices[0]));

  while (1) {
    // this way, it will point to the NULL character at the end
    // as well as not count it as a written index
    uint16_t bytes_read = uart_ngetc(cmd, cmd_index, TX_BUF_SIZE, TX_BUF_SIZE);
    if (bytes_read == (uint16_t) -1) {
      uart_puts(COMMAND_LONG);
      cmd_index = 0;
      continue;
    }
    cmd_index += bytes_read;
    if (cmd_index > 0 && cmd[cmd_index - 1] == '\r') {
      if (cmd[0] == 'r' || cmd[0] == 'g' || cmd[0] == 'b') {
        // Must destroy this device before we get rid of it
        uart_puts((unsigned char *) devices[0].destroy(devices[0]));
        // type_num will still be 0 since that was the type_num of the first
        // actuator to be created
        // R = PA0, G = PA1, B = PA2
        create_device(0, (char *) cmd);
        devices[0].init(devices[0]);
        cmd_index = 0;
        continue;
      }
      if (strncmp((char *) cmd, "destroy", 7) == 0) {
        uart_puts((unsigned char *) devices[0].destroy(devices[0]));
        cmd_index = 0;
        continue;
      }

      if (strncmp((char *) cmd, "init", 4) == 0) {
        uart_puts((unsigned char *) devices[0].init(devices[0]));
        cmd_index = 0;
        continue;
      }

      uart_puts((unsigned char *) devices[0].write(devices[0], (void *) cmd));
      //uart_putc('\n');
      cmd_index = 0; // restart the index to start a new command
    }
  }
  return 0;
}
