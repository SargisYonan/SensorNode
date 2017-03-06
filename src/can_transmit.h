#ifndef CAN_TRANSMIT_H_
#define CAN_TRANSMIT_H_

#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/interrupt.h>

#ifndef CAN_TRANSMIT_MAX 
#define CAN_TRANSMIT_MAX 10
#endif

#define CAN_TRANSMIT_IDENTIFIER_STRING "CAN_TRANSMIT"

typedef Module Can_Transmit 

Can_Transmit new_can_transmit(void);

void *can_Transmit_init(Can_Transmit); // default init function
void *can_transmit_read(Can_Transmit); // default read function
void *can_transmit_write(Can_Transmit, void *); // default write function
void *can_transmit_destroy(Can_Transmit); // default destroy function

#endif
