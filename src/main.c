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

// TODO: handle holes in device array (use valid bits)
void create_device(uint8_t type_num, uint8_t pin_map_index,
    uint8_t reg_bit) {
  devices[devices_count++] = type_num_to_create_function_map[type_num](type_num,
      port_map[pin_map_index], pin_map[pin_map_index], ddr_map[pin_map_index],
      reg_bit);

  /* // back in the hardcoding days
     devices[device] = type_num_to_create_function_map[0] // type for actuator = 0
     (0, &PORTA, &PINA, &DDRA, cmd[0] == 'r' ? PA0 : cmd[0] == 'g' ? PA1 : PA2);

  devices[device] = type_num_to_create_function_map[1] // type for temp sensor
    (1, &PORTC, &PINC, &DDRC,
     cmd[0] ? PC0 : PC0); // the first argument is index of type array
     */
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
  add_to_resolvers(num_types++, TEMP_SENSOR_IDENTIFIER_STRING,
      &new_temp_sensor);

  sei();

  uart_init();

  unsigned char cmd[TX_BUF_SIZE + 1]; // max amount written by uart_ngetc()
  uint16_t cmd_index = 0;

  // temporary hardcode
  create_device(resolve_type_string_to_num(TEMP_SENSOR_IDENTIFIER_STRING),
      2, 0); // PC0 = pin37
  //uart_puts(devices[0].init(devices[0])); // disconnected right now

  while (1) {
    // this way, it will point to the NULL character at the end
    // as well as not count it as a written index
    uint16_t bytes_read = uart_ngetc(cmd, cmd_index, RX_BUF_SIZE, RX_BUF_SIZE);
    if (bytes_read == (uint16_t) -1) {
      uart_puts(COMMAND_LONG);
      cmd_index = 0;
      continue;
    }
    cmd_index += bytes_read;

    if (cmd_index > 0 && cmd[cmd_index - 1] == '\r') {
      Parser p = parse_cmd((char *) cmd);
      //uart_printf("CMD: %c, ret_str: %s\r\n", p.cmd, p.ret_str);
      switch(p.cmd) { // This assumes the string has been parsed
        case CHAR_CREATE:
          uart_printf("Create has not yet been implemented\r\n");
          break;
        case CHAR_INIT:
          if (p.device_index >= devices_count) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          uart_puts(devices[p.device_index].init(devices[p.device_index]));
          break;
        case CHAR_READ:
          if (p.device_index >= devices_count) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          uart_puts(devices[p.device_index].read(devices[p.device_index]));
          break;
        case CHAR_WRITE:
          if (p.device_index >= devices_count) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          uart_puts(devices[p.device_index].write(devices[p.device_index],
                (void *) p.ret_str));
          break;
        case CHAR_DESTROY:
          if (p.device_index >= devices_count) {
            uart_printf("%d: Invalid Device Specified\r\n", p.device_index);
            break;
          }
          uart_puts(devices[p.device_index].destroy(devices[p.device_index]));
          break;
        case CHAR_KILL:
          uart_printf("Kill has not yet been implemented\r\n");
          break;
        case CHAR_MAP:
          uart_printf("There are %d devices installed:\r\n", devices_count);
          for (int i = 0; i < devices_count; i++) {
            uart_printf("\tdevice %d: type=%s, type_count=%d, pin=%s\r\n",
                i, type_num_to_string_map[devices[i].type_num],
                devices[i].index, "TODO");
          }
          uart_printf("\r\n");
          break;
        default:
          uart_printf("Invalid Command\r\n");
          break;
      }
      cmd_index = 0;
      continue;

      if(cmd[0] == 'm') { // TODO: parser stuff here
        // TODO: mapping function
        uart_puts((unsigned char *) "Mapping function not yet ready\r\n");
        cmd_index = 0; // this is necessary to clear the string
        continue;
      }
      /*
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
      */
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

      uart_puts((unsigned char *) devices[0].read(devices[0]));
      //uart_putc('\n');
      cmd_index = 0; // restart the index to start a new command
    }
  }
  return 0;
}
