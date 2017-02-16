#ifndef MODULE_H_
#define MODULE_H_

#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>

typedef struct Module {
  uint8_t type_num; // determined through runtime based on order of creation
  uint8_t index; // order of module of type(type_num) to be created (start at 0)
  void *(*init)(void); // init function
  void *(*read)(void); // read function
  void *(*write)(void *); // write function
} Module;

Module new_module(void); // all modules are created with this function

void *module_init(void); // default init function
void *module_read(void); // default read function
void *module_write(void *); // default write function

#endif
