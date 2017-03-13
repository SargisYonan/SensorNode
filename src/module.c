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
  m.type_str = MODULE_IDENTIFIER_STRING;
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
  char out_str[128];
  sprintf(out_str, "Initialized type: %s\r\n", m.type_str);
  const char *ret_str = (const char *) out_str;
  return (void *) ret_str;
}

void *module_read(Module m) {
  char out_str[128];
  sprintf(out_str, "Read type: %s\r\n", m.type_str);
  const char *ret_str = (const char *) out_str;
  return (void *) ret_str;
}

void *module_write(Module m, void *write_data) {
  char out_str[256];
  sprintf(out_str, "Write to type: %s\r\nWith %s\r\n", m.type_str,
      (char *)write_data);
  const char *ret_str = (const char *) out_str;
  return (void *) ret_str;
}

void *module_destroy(Module m) {
  char out_str[128];
  sprintf(out_str, "Destroyed type: %s\r\n", m.type_str);
  const char *ret_str = (const char *) out_str;
  return (void *) ret_str;
}
