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

#define LED PB7
#define LEDDDR DDRB
#define LEDPORT PORTB

#define MAX_DEVICES 100

#define COMMAND_LONG (unsigned char *) \
  "COMMAND EXCEEDED TX_BUF_SIZE Characters\r\n"

int main(void){
  sei();
  
  uart_init();

  Module devices[MAX_DEVICES];
  uint8_t devices_count = 0;
  // TODO: resolver for TYPE_STRING to TYPE_NUM
  uint8_t type_cur_num = 0; // a number tracking the next device type to assign
  unsigned char cmd[TX_BUF_SIZE + 1]; // max amount written by uart_ngetc()
  uint16_t cmd_index = 0;

  // temporary hardcode
  // TODO: resolver for ID to new_DEVICE function
  devices[devices_count++] = new_actuator(type_cur_num, &PORTA, &PINA, &DDRA,
      PA0);
  // official method for incrementing type_cur_num
  // This is because you don't know if a device of this type exists yet
  if (devices[0].type_num == type_cur_num) type_cur_num++;
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
        devices[0] = new_actuator(type_cur_num, &PORTA, &PINA, &DDRA,
            cmd[0] == 'r' ? PA0 : cmd[0] == 'g' ? PA1 : PA2);
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
