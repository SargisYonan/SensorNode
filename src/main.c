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

typedef Module (* NEW_DEVICE_FUNC_TYPE) (uint8_t, Module);

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
int8_t resolve_type_string_to_num(char *target) {
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
  uart_printf("initializing the string for type %d as %s\r\n", type_num,
      (char *) type_num_to_string_map[type_num]);
}

// create a device on the next available spot in the array
// create by setting valid bit to 1, calling appropriate create function
//  and increasing the devices_count.
// also call init
void create_device(uint8_t type_num, const uint8_t *pin_map_index,
    const uint8_t *reg_bit, uint8_t pin_count) {
  if (devices_count == MAX_DEVICES) return;
  Module m = new_module();
  m.pin_count = pin_count;
  for (uint8_t i = 0; i < pin_count; i++) { // note that pin_count will be at most 8
    uart_printf("Kill me: %d\r\np_m_ind=%d, reg_bit=%d\r\n", i,
        pin_map_index[i], reg_bit[i]);
    uart_flushTX();
    m.port[i] = port_map[pin_map_index[i]];
    m.pin[i] = pin_map[pin_map_index[i]];
    m.ddr[i] = ddr_map[pin_map_index[i]];
    m.reg_bit[i] = reg_bit[i];
  }
  for (uint8_t i = 0; 1; i++) {
    if (i == MAX_DEVICES) {
      uart_printf(
          "How did you manage to iterate to the end of the device array?\r\n");
      break;
    }
    if (devices_valid[i] == 0) {
      devices[i] =
        type_num_to_create_function_map[type_num](type_num, m);
      devices_valid[i] = 1;
      uart_printf("Created new device of type %s on index %d with %d pins\r\n",
          (char *) type_num_to_string_map[type_num], i, pin_count);
      char buf[128];
      strcpy_P(buf, (PGM_P) pgm_read_word(devices[i].init(devices[i])));
      uart_printf(buf);
      devices_count++;
      break;
    }
  }
}

// destroy the device and set it's valid bit to 0
// also decrease device count
void remove_device(uint8_t device_index) {
  if (device_index >= MAX_DEVICES) return;
  char buf[128];
  strcpy_P(buf, (PGM_P) pgm_read_word(devices[device_index].
        destroy(devices[device_index])));
  uart_printf(buf);
  uart_printf("Removed device %d\r\n", device_index);
  devices_valid[device_index] = 0;
  devices_count--;
}

int main(void){

  sei();

  uart_init();

  for (int i = 0; i < MAX_DEVICES; i++)
    devices_valid[i] = 0; // initialize to 0

  // TODO: based off #defines of whether these exist in the first place
  add_to_resolvers(num_types++, ACTUATOR_IDENTIFIER_STRING, &new_actuator);
  //uart_printf("GOD, IF YOU EXIST, PLZ HELP: %s\r\n", (char *) type_num_to_string_map[0]);
  //add_to_resolvers(num_types++, TEMP_SENSOR_IDENTIFIER_STRING,
  //    &new_temp_sensor);
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
      char buf[128];
      //uart_printf("CMD: %c, ret_str: %s\r\n", p.cmd, p.ret_str); // debug
      switch(p.cmd) { // This assumes the string has been parsed
        case CHAR_CREATE:
          {
            int8_t type = resolve_type_string_to_num((char *) p.ret_str);
            if (type == -1) {
              uart_printf("%s: Invalid Type String\r\n", p.ret_str);
              break;
            }
            create_device(type, p.address_index, p.reg_bit, p.pin_count);
          }
          break;
        case CHAR_INIT:
          if (p.device_index >= MAX_DEVICES ||
              devices_valid[p.device_index] == 0) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          strcpy_P(buf, (PGM_P) pgm_read_word(devices[p.device_index].
                init(devices[p.device_index])));
          uart_printf(buf);
          break;
        case CHAR_READ:
          if (p.device_index >= MAX_DEVICES ||
              devices_valid[p.device_index] == 0) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          strcpy_P(buf, (PGM_P) pgm_read_word(devices[p.device_index].
                read(devices[p.device_index])));
          uart_printf(buf);
          break;
        case CHAR_WRITE:
          if (p.device_index >= MAX_DEVICES ||
              devices_valid[p.device_index] == 0) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          strcpy_P(buf, (PGM_P) pgm_read_word(devices[p.device_index].
                write(devices[p.device_index], (char *) p.ret_str)));
          uart_printf(buf);
          break;
        case CHAR_DESTROY:
          if (p.device_index >= MAX_DEVICES ||
              devices_valid[p.device_index] == 0) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          strcpy_P(buf, (PGM_P) pgm_read_word(devices[p.device_index].
                destroy(devices[p.device_index])));
          uart_printf(buf);
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
          uart_printf("There are %d different types of devices available\r\n",
              num_types);
          for (uint8_t i = 0; i < num_types; i++) {
            uart_printf("%d: %s\r\n", i, (char *) type_num_to_string_map[i]);
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
