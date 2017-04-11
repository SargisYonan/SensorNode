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
  m.pin_count = 0;
  m.init = &module_init;
  m.read = &module_read;
  m.write = &module_write;
  m.destroy = &module_destroy;
  // this will return the address of the index of the array then increment count
  return m;
}

PGM_P module_init(Module m) {
  char out_str[128];
  sprintf(out_str, "Initialized type: %d\r\n", m.type_num);
  PGM_P ret_str = out_str;
  return ret_str;
}

PGM_P module_read(Module m) {
  char out_str[128];
  sprintf(out_str, "Read type: %d\r\n", m.type_num);
  PGM_P ret_str = out_str;
  return ret_str;
}

PGM_P module_write(Module m, char *write_data) {
  char out_str[256];
  sprintf(out_str, "Write to type: %d\r\nWith data: %s\r\n", m.type_num,
      (char *)write_data);
  PGM_P ret_str = out_str;
  return ret_str;
}

PGM_P module_destroy(Module m) {
  char out_str[128];
  sprintf(out_str, "Destroyed type: %d\r\n", m.type_num);
  PGM_P ret_str = out_str;
  return ret_str;
}
