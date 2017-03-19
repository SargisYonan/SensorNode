#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "OneWire.h"
#include "uart.h"
#include "parser.h"
#include "module.h"
#include "actuator.h"
#include "common.h"

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
// port map
volatile uint8_t *port_map[8] = {&PORTA, &PORTB, &PORTC, &PORTD, &PORTE, &PORTF,
  &PORTG, &PORTH};
// pin map
volatile uint8_t *pin_map[8] = {&PINA, &PINB, &PINC, &PIND, &PINE, &PINF,
  &PING, &PINH};
// ddr map
volatile uint8_t *ddr_map[8] = {&DDRA, &DDRB, &DDRC, &DDRD, &DDRE, &DDRF,
  &DDRG, &DDRH};
uint8_t num_types = 0; // track the number of different types
uint8_t device_next; // next index to create a device in
uint8_t devices_count = 0;

// resolve type string to num (just do a linear search)
// return -1 if name not found
uint8_t resolve_type_string_to_num(char *target) {
  for (uint8_t i = 0; i < num_types; i++) {
    if (strlen(target) == strlen((char *) type_num_to_string_map[i]) &&
        strcmp(target, (char *) type_num_to_string_map[i]) == 0) {
      return i;
    }
  }
  return -1;
}

// called initially for every type of module
void add_to_resolvers(uint8_t type_num, char const *type_string,
    NEW_DEVICE_FUNC_TYPE new_device) {
  type_num_to_string_map[type_num] = type_string;
  type_num_to_create_function_map[type_num] =
    new_device;
}

// create a device on the next available spot in the array
// create by setting valid bit to 1, calling appropriate create function
//  and increasing the devices_count.
// also call init
void create_device(uint8_t type_num, uint8_t pin_map_index,
    uint8_t reg_bit) {
  if (devices_count == MAX_DEVICES) return;
  for (uint8_t i = 0; 1; i++) { // if the above is false, there is no condition here
    if (devices_valid[i] == 0) {
      devices[i] =
        type_num_to_create_function_map[type_num](type_num,
            port_map[pin_map_index], pin_map[pin_map_index],
            ddr_map[pin_map_index], reg_bit);
      devices_valid[i] = 1;
      uart_printf("Created new device of type %s on index %d with %s%d%s%d\r\n",
          (char *) type_num_to_string_map[type_num], i,
          "a pin_map_index of ", pin_map_index,
          " and a reg_bit of ", reg_bit);
      uart_printf(devices[i].init(devices[i]));
      devices_count++;
      break;
    }
  }
}

// destroy the device and set it's valid bit to 0
// also decrease device count
void remove_device(uint8_t device_index) {
  if (device_index >= MAX_DEVICES) return;
  uart_printf(devices[device_index].destroy(devices[device_index]));
  uart_printf("Removed device %d\r\n", device_index);
  devices_valid[device_index] = 0;
  devices_count--;
}

int main(void){

  for (int i = 0; i < MAX_DEVICES; i++)
    devices_valid[i] = 0; // initialize to 0

  // TODO: perhaps make the function handle the incrementation and global
  // access of num_types?
  add_to_resolvers(num_types++, ACTUATOR_IDENTIFIER_STRING, &new_actuator);
  add_to_resolvers(num_types++, TEMP_SENSOR_IDENTIFIER_STRING,
      &new_temp_sensor);

  sei();

  uart_init();

  unsigned char cmd[TX_BUF_SIZE + 1]; // max amount written by uart_ngetc()
  uint16_t cmd_index = 0;

  /*
  // temporary hardcode
  create_device(resolve_type_string_to_num(ACTUATOR_IDENTIFIER_STRING),
  0, 0); // PA0 = pin22
  create_device(resolve_type_string_to_num(ACTUATOR_IDENTIFIER_STRING),
  0, 1); // PA1 = pin23
  create_device(resolve_type_string_to_num(ACTUATOR_IDENTIFIER_STRING),
  0, 2); // PA2 = pin24
  create_device(resolve_type_string_to_num(TEMP_SENSOR_IDENTIFIER_STRING),
  2, 0); // PC0 = pin37
  */

  while (1) {
    // this way, cmd_index will point to the NULL character at the end
    // as well as not count it as a written index (if you treat cmd_index as
    // the size)
    uint16_t bytes_read = uart_ngetc(cmd, cmd_index, RX_BUF_SIZE, RX_BUF_SIZE);
    if (bytes_read == (uint16_t) -1) {
      uart_puts(COMMAND_LONG);
      cmd_index = 0;
      continue;
    }
    cmd_index += bytes_read;

    if (cmd_index > 0 && cmd[cmd_index - 1] == '\r') {
      Parser p = parse_cmd((char *) cmd);
      //uart_printf("CMD: %c, ret_str: %s\r\n", p.cmd, p.ret_str); // debug
      switch(p.cmd) { // This assumes the string has been parsed
        case CHAR_CREATE: // TODO: multi pin per module functionality
          {
            int type = resolve_type_string_to_num((char *) p.ret_str);
            if (type == -1) {
              uart_printf("%s: Invalid Type String\r\n", p.ret_str);
              break;
            }
            create_device(type, p.address_index, p.reg_bit);
          }
          break;
        case CHAR_INIT:
          if (p.device_index >= MAX_DEVICES ||
              devices_valid[p.device_index] == 0) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          uart_puts(devices[p.device_index].init(devices[p.device_index]));
          break;
        case CHAR_READ:
          if (p.device_index >= MAX_DEVICES ||
              devices_valid[p.device_index] == 0) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          uart_puts(devices[p.device_index].read(devices[p.device_index]));
          break;
        case CHAR_WRITE:
          if (p.device_index >= MAX_DEVICES ||
              devices_valid[p.device_index] == 0) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          uart_puts(devices[p.device_index].write(devices[p.device_index],
                (void *) p.ret_str));
          break;
        case CHAR_DESTROY:
          if (p.device_index >= MAX_DEVICES ||
              devices_valid[p.device_index] == 0) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          uart_puts(devices[p.device_index].destroy(devices[p.device_index]));
          break;
        case CHAR_KILL:
          if (p.device_index >= MAX_DEVICES ||
              devices_valid[p.device_index] == 0) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          remove_device(p.device_index);
          break;
        case CHAR_MAP:
          uart_printf("There are %d devices installed:\r\n", devices_count);
          uint8_t count = devices_count;
          for (uint8_t i = 0; count > 0; i++) {
            if (devices_valid[i] == 0) continue;
            uart_printf("\tdevice %d: type=%s, type_count=%d, pin=%s\r\n",
                i, type_num_to_string_map[devices[i].type_num],
                devices[i].index, "TODO");
            count--;
          }
          uart_printf("\r\n");
          break;
        default:
          uart_printf("Invalid Command\r\n");
          break;
      }
      cmd_index = 0;
      continue;
    }
  }
  return 0;
}
