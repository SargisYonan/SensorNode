#include "Actuator.h"

Actuator actuators[ACTUATOR_MAX]; // array of Actuators (treat as *)
uint8_t actuator_count = 0;
uint8_t actuator_type_num = -1; // needs to be set on first creation of Actuator

// sets an index of the actuator module array to be the new actuator's info
// also sets the fields accordingly
// RETURNS:
// the address within the array that points to the pointer to the struct
// or NULL if no new block was allocated
void *new_actuator(uint8_t cur_type_num) {
  if (actuator_count >= ACTUATOR_MAX) {
    return NULL; // our way of communicating that you couldn't add another
  }
  if (actuator_count == 0) {
    actuator_type_num = cur_type_num;
  }
  actuators[actuator_count].type_num = actuator_type_num;
  actuators[actuator_count].index = actuator_count;
  // this will return the address of the index of the array then increment count
  return (void *) (actuators + actuator_count++);
}

char *actuator_write() {
  
}
