#include "module.h"

#include "uart.h"

#define DEFAULT_TYPE_NUM -1 // basically means it's unset

// sets fields of a new module to defaults and returns it
// meant to be called from a specific module's new_$(MODULE_NAME)() function
// RETURNS:
// a new module
Module new_module() {
  Module m;
  m.type_num = DEFAULT_TYPE_NUM;
  m.pin_count = 0;
  m.init = &module_init;
  m.read = &module_read;
  m.write = &module_write;
  m.destroy = &module_destroy;
  // this will return the address of the index of the array then increment count
  return m;
}

void module_init(Module m) {
  uart_printf("Initialized type: %d\r\n", m.type_num);
}

void module_read(Module m, char *read_data, uint16_t max_bytes) {
  uart_printf("Read type: %d\r\n", m.type_num);
  snprintf(read_data, max_bytes, "Read type: %d\r\n", m.type_num);
}

void module_write(Module m, char *write_data, uint16_t max_bytes) {
  if (max_bytes) {} // stop complaining with warnings
  uart_printf("Write to type: %d\r\nWith data: %s\r\n", m.type_num,
      (char *)write_data);
}

void module_destroy(Module m) {
  uart_printf("Destroyed type: %d\r\n", m.type_num);
}
