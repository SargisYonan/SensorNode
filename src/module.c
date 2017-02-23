#include "module.h"

#define DEFAULT_TYPE_NUM -1 // basically means it's unset
#define INDEX_INIT -1 // basically means index undefined

// sets fields of a new module to defaults and returns it
// meant to be called from a specific module's new_$(MODULE_NAME)() function
// RETURNS:
// a new module
Module new_module() {
  Module m;
  m.type_num = DEFAULT_TYPE_NUM;
  m.index = INDEX_INIT;
  m.port = NULL;
  m.pin = NULL;
  m.ddr = NULL;
  m.reg_bit = INDEX_INIT;
  m.init = &module_init;
  m.read = &module_read;
  m.write = &module_write;
  m.destroy = &module_destroy;
  // this will return the address of the index of the array then increment count
  return m;
}

void *module_init(Module m) {
  return NULL;
}

void *module_read(Module m) {
  return NULL;
}

void *module_write(Module m, void *write_data) {
  return (void *) write_data;
}

void *module_destroy(Module m) {
  return NULL;
}
