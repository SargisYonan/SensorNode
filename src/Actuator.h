#ifndef ACTUATOR_H_
#define ACTUATOR_H_

#ifndef ACTUATOR_MAX 
#define ACTUATOR_MAX 10
#endif

#define ACTUATOR_IDENTIFIER_STRING "ACTUATOR"

typedef struct Actuator {
  uint8_t type_num; // determined through runtime based on order of creation
  uint8_t index; // index in the actuators array
  void *init; // init function
  void *read; // read function
  void *write; // write function
} Actuator;

void *new_actuator(uint8_t);

char *actuator_write(void *);

#endif
