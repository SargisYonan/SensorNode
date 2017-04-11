#ifndef ACTUATOR_H_
#define ACTUATOR_H_

#include <string.h>
#include "module.h"

#ifndef ACTUATOR_MAX 
#define ACTUATOR_MAX 10
#endif

#define ACTUATOR_IDENTIFIER_STRING "ACTUATOR"

typedef Module Actuator;

Actuator new_actuator(uint8_t, Actuator);

PGM_P actuator_init(Actuator a);

PGM_P actuator_write(Actuator a, char *);

PGM_P actuator_destroy(Actuator a);

#endif
