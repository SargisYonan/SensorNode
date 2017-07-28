#ifndef CAN_BUS_TRANSMIT_H_
#define CAN_BUS_TRANSMIT_H_

#include "module.h"

#ifndef CAN_BUS_MAX
#define CAN_BUS_MAX 10
#endif

#define CAN_BUS_IDENTIFIER_STRING "CAN_BUS"

typedef Module Can_Bus;

Can_Bus new_can_bus(uint8_t, Can_Bus);

void can_bus_init(Can_Bus); // default init function
void can_bus_read(Can_Bus, char *, uint16_t); // default read function
void can_bus_write(Can_Bus, char *, uint16_t); // default write function
void can_bus_destroy(Can_Bus); // default destroy function

#endif
