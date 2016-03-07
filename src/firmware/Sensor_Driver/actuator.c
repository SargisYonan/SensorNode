#include "actuator.h"

void TurnOffPin(void)
{
	if (IS_SET(ACT_PORT_IN, ACT_PIN))
	{
		CLEAR_BIT(ACT_PORT_OUT, ACT_PIN);
	}
}

void TurnOnPin(void)
{
	if (!(IS_SET(ACT_PORT_IN, ACT_PIN)))
	{
		SET_BIT(ACT_PORT_OUT, ACT_PIN);
	}
}

void PinInit(void)
{
   	CLEAR_DDR();
   	SET_BIT(ACT_DDR, ACT_PIN);
}