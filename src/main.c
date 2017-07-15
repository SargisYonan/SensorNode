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
#include "light_sensor.h"
#include "humidity_sensor.h"
#include "fona.h"

#define MAX_DEVICES 100

#define COMMAND_LONG (unsigned char *) \
  "COMMAND EXCEEDED TX_BUF_SIZE Characters\r\n"

typedef Module (* NEW_DEVICE_FUNC_TYPE) (uint8_t, Module);

Module devices[MAX_DEVICES]; // all devices currently running
uint8_t devices_valid[MAX_DEVICES]; // device exists if its index contains 1

#ifdef ACTUATOR_H_
  static const char actuator_str[] PROGMEM = ACTUATOR_IDENTIFIER_STRING;
#endif
#ifdef ONEWIRE_H
  static const char temp_sens_str[] PROGMEM = TEMP_SENSOR_IDENTIFIER_STRING;
#endif
#ifdef LIGHT_SENSOR_H_
  static const char light_sensor_str[] PROGMEM = LIGHT_SENSOR_IDENTIFIER_STRING;
#endif
#ifdef HUMIDITY_SENSOR_H_
  static const char humidity_sensor_str[] PROGMEM = HUMIDITY_SENSOR_IDENTIFIER_STRING;
#endif
#ifdef FONA_H_
  static const char fona_str[] PROGMEM = FONA_IDENTIFIER_STRING;
#endif

// type i points to a corresponding string
static PGM_P type_num_to_string_map[MAX_DEVICES] = {
#ifdef ACTUATOR_H_
  actuator_str,
#endif
#ifdef ONEWIRE_H
  temp_sens_str,
#endif
#ifdef LIGHT_SENSOR_H_
  light_sensor_str,
#endif
#ifdef HUMIDITY_SENSOR_H_
  humidity_sensor_str,
#endif
#ifdef FONA_H_
  fona_str,
#endif
};
// type i points to a corresponding creation function
static NEW_DEVICE_FUNC_TYPE type_num_to_create_function_map [MAX_DEVICES] = {
#ifdef ACTUATOR_H_
  &new_actuator,
#endif
#ifdef ONEWIRE_H
  &new_temp_sensor,
#endif
#ifdef LIGHT_SENSOR_H_
  &new_light_sensor,
#endif
#ifdef HUMIDITY_SENSOR_H_
  &new_humidity_sensor,
#endif
#ifdef FONA_H_
  &new_fona,
#endif
};
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
    if (strlen(target) == strlen_P(type_num_to_string_map[i]) &&
        strcmp_P(target, type_num_to_string_map[i]) == 0) {
      return i;
    }
  }
  return -1;
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
    m.port[i] = port_map[pin_map_index[i]];
    m.pin[i] = pin_map[pin_map_index[i]];
    m.ddr[i] = ddr_map[pin_map_index[i]];
    m.reg_bit[i] = reg_bit[i];
  }
  for (uint8_t i = 0; 1; i++) {
    if (devices_valid[i] == 0) {
      devices[i] =
        type_num_to_create_function_map[type_num](type_num, m);
      devices_valid[i] = 1;
      uart_printf("Created new device of type ");
      uart_puts_P(type_num_to_string_map[type_num]);
      uart_printf(" on index %d with %d pins\r\n",
          i, pin_count);
      devices[i].init(devices[i]);
      devices_count++;
      break;
    }
  }
}

// destroy the device and set it's valid bit to 0
// also decrease device count
void remove_device(uint8_t device_index) {
  if (device_index >= MAX_DEVICES) return;
  devices[device_index].destroy(devices[device_index]);
  uart_printf("Removed device %d\r\n", device_index);
  devices_valid[device_index] = 0;
  devices_count--;
}

int main(void){

  sei();

  uart_init(19200);

  for (int i = 0; i < MAX_DEVICES; i++)
    devices_valid[i] = 0; // initialize to 0

  // initialize num_types to number of types
  for (num_types = 0; type_num_to_string_map[num_types] != NULL; num_types++);

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
          devices[p.device_index].init(devices[p.device_index]);
          break;
        case CHAR_READ:
          if (p.device_index >= MAX_DEVICES ||
              devices_valid[p.device_index] == 0) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          devices[p.device_index].read(devices[p.device_index]);
          break;
        case CHAR_WRITE:
          if (p.device_index >= MAX_DEVICES ||
              devices_valid[p.device_index] == 0) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          devices[p.device_index].write(devices[p.device_index],
              (char *) p.ret_str);
          break;
        case CHAR_DESTROY:
          if (p.device_index >= MAX_DEVICES ||
              devices_valid[p.device_index] == 0) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          devices[p.device_index].destroy(devices[p.device_index]);
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
          uart_flushTX();
          uint8_t count = devices_count;
          for (uint8_t i = 0; count > 0; i++) {
            if (devices_valid[i] == 0) continue;
            uart_printf("\tdevice %d: type=", i);
            uart_puts_P(type_num_to_string_map[devices[i].type_num]);
            uart_printf(" pin_count=%d\r\n",
                devices[i].pin_count);
            uart_flushTX();
            count--;
          }
          uart_printf("\r\n");
          uart_printf("There are %d different types of devices available\r\n",
              num_types);
          uart_flushTX();
          for (uint8_t i = 0; i < num_types; i++) {
            uart_printf("%d: ", i);
            uart_puts_P(type_num_to_string_map[i]);
            uart_printf("\r\n");
            uart_flushTX();
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
