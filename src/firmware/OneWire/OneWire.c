 /* Directions
 * ----------
 *
 * 1. Modify F_CPU, DALLAS_PORT, DALLAS_DDR, DALLAS_PORT_IN, DALLAS_PIN, and
 *    DALLAS_NUM_DEVICES to match your application.
 * 2. In your code, first run dallas_search_identifiers() to populate the 
 *    the list of identifiers with the devices on your bus.
 */


#include "OneWire.h"


DALLAS_IDENTIFIER_LIST_t identifier_list;


// Identifier routine return codes //

#define DALLAS_IDENTIFIER_NO_ERROR 0x00
#define DALLAS_IDENTIFIER_DONE 0x01
#define DALLAS_IDENTIFIER_SEARCH_ERROR 0x02


// Private function prototypes //

static uint8_t dallas_discover_identifier(DALLAS_IDENTIFIER_t *,
		DALLAS_IDENTIFIER_t *);

// Functions //

void dallas_write(uint8_t bit) {
	if (bit == 0x00) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			// Configure the pin as an output.
			DALLAS_DDR |= _BV(DALLAS_PIN);

			// Pull the bus low.
			DALLAS_PORT &= ~_BV(DALLAS_PIN);

			// Wait the required time.
			_delay_us(DALLAS_TIME_ZERO);

			// Release the bus.
			DALLAS_PORT |= _BV(DALLAS_PIN);

			// Let the rest of the time slot expire.
			_delay_us(DALLAS_TIME_FRAME - DALLAS_TIME_ZERO);
		}
	} else {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			// Configure the pin as an output.
			DALLAS_DDR |= _BV(DALLAS_PIN);

			// Pull the bus low.
			DALLAS_PORT &= ~_BV(DALLAS_PIN);

			// Wait the required time.
			_delay_us(DALLAS_TIME_ONE);

			// Release the bus.
			DALLAS_PORT |= _BV(DALLAS_PIN);

			// Let the rest of the time slot expire.
			_delay_us(DALLAS_TIME_FRAME - DALLAS_TIME_ZERO);
		}
	}
}

uint8_t dallas_read(void) {
	uint8_t reply;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		// Configure the pin as an output.
		DALLAS_DDR |= _BV(DALLAS_PIN);

		// Pull the bus low.
		DALLAS_PORT &= ~_BV(DALLAS_PIN);

		// Wait the required time.
		_delay_us(2);

		// Configure as input.
		DALLAS_DDR &= ~_BV(DALLAS_PIN);

		// Wait for a bit.
		_delay_us(11);

		if ((DALLAS_PORT_IN & _BV(DALLAS_PIN)) == 0x00) {
			reply = 0x00;
		} else {
			reply = 0x01;
		}

		// Let the rest of the time slot expire.
		_delay_us(47);
	}

	return reply;
}

// Resets the bus and returns 0x01 if a slave indicates present, 0x00 otherwise.
uint8_t dallas_reset(void) {
	uint8_t reply;

	// Reset the slave_reply variable.
	reply = 0x00;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{

		// Configure the pin as an output.
		DALLAS_DDR |= _BV(DALLAS_PIN);

		// Pull the bus low.
		DALLAS_PORT &= ~_BV(DALLAS_PIN);

		// Wait the required time.
		_delay_us(DALLAS_TIME_RESET); // 500 uS

		// Switch to an input, enable the pin change interrupt, and wait.
		DALLAS_DDR &= ~_BV(DALLAS_PIN);

		_delay_us(DALLAS_TIME_PRESENCE);

		if ((DALLAS_PORT_IN & _BV(DALLAS_PIN)) == 0x00) {
			reply = 0x01;
		}

		//wait 8xframe to balance bus state
		_delay_us(DALLAS_TIME_RESET);
	}

	return reply;
}

void dallas_write_byte(uint8_t byte) {
	uint8_t position;

	for (position = 0x00; position < 0x08; position++) {
		dallas_write(byte & 0x01);

		byte = (byte >> 1);
	}

	_delay_us(DALLAS_TIME_COMMAND_REST);

}

uint8_t dallas_read_byte(void) {
	uint8_t byte;
	uint8_t position;

	byte = 0x00;

	for (position = 0x00; position < 0x08; position++) {
		byte += (dallas_read() << position);
	}

	return byte;
}

// Uses the uC to power the bus.
void dallas_drive_bus(void) {
	// Configure the pin as an output.
	DALLAS_DDR |= _BV(DALLAS_PIN);

	// Set the bus high.
	DALLAS_PORT |= _BV(DALLAS_PIN);
}

void dallas_match_rom(DALLAS_IDENTIFIER_t * identifier) {
	uint8_t identifier_bit;
	uint8_t current_byte;
	uint8_t current_bit;

	dallas_reset();
	dallas_write_byte(MATCH_ROM_COMMAND);

	for (identifier_bit = 0x00; identifier_bit < DALLAS_NUM_IDENTIFIER_BITS;
			identifier_bit++) {
		current_byte = identifier_bit / 8;
		current_bit = identifier_bit - (current_byte * 8);

		dallas_write(identifier->identifier[current_byte] & _BV(current_bit));
	}
}

void dallas_skip_rom(void) {
	//dallas_reset();
	dallas_write_byte(SKIP_ROM_COMMAND);
}

uint8_t dallas_search_identifiers(void) {
	uint8_t current_device;
	uint8_t return_code;

	for (current_device = 0x00; current_device < DALLAS_NUM_DEVICES;
			current_device++) {
		if (current_device == 0x00) {
			return_code = dallas_discover_identifier(
					&identifier_list.identifiers[current_device], 0x00);
		} else {
			return_code = dallas_discover_identifier(
					&identifier_list.identifiers[current_device],
					&identifier_list.identifiers[current_device - 1]);
		}

		if (return_code == DALLAS_IDENTIFIER_DONE) {
			identifier_list.num_devices = current_device + 0x01;
			return 0x00;
		} else if (return_code == DALLAS_IDENTIFIER_SEARCH_ERROR) {
			return 0x01;
		}
	}

	return 0x02;
}

DALLAS_IDENTIFIER_LIST_t * get_identifier_list(void) {
	return &identifier_list;
}

static uint8_t dallas_discover_identifier(
		DALLAS_IDENTIFIER_t * current_identifier,
		DALLAS_IDENTIFIER_t * last_identifier) {
	uint8_t identifier_bit;
	uint8_t received_two_bits;
	uint8_t current_bit;
	uint8_t current_byte;
	uint8_t identifier_diverged;

	identifier_diverged = 0x00;
	identifier_bit = 0x00;

	dallas_reset();
	dallas_write_byte(SEARCH_ROM_COMMAND);

	for (identifier_bit = 0; identifier_bit < DALLAS_NUM_IDENTIFIER_BITS;
			identifier_bit++) {
		received_two_bits = (dallas_read() << 1);
		received_two_bits += dallas_read();

		current_byte = identifier_bit / 8;
		current_bit = identifier_bit - (current_byte * 8);

		if (received_two_bits == 0x02) {
			// All devices have a 1 at this position.

			current_identifier->identifier[current_byte] += (1 << current_bit);

			dallas_write(0x01);
		} else if (received_two_bits == 0x01) {
			// All devices have a 0 at this position.

			dallas_write(0x00);
		} else if (received_two_bits == 0x00) {
			if ((identifier_diverged == 0x00) && (last_identifier != 0x00)) {
				identifier_diverged = 0x01;

				if ((last_identifier->identifier[current_byte]
						& _BV(current_bit)) == 0x00) {
					// Then we choose 1.

					current_identifier->identifier[current_byte] += (1
							<< current_bit);

					dallas_write(0x01);
				} else {
					// Otherwise 0.

					dallas_write(0x00);
				}
			} else {
				// We'll go with 0.
				dallas_write(0x00);
			}
		} else {
			// Error
			return DALLAS_IDENTIFIER_SEARCH_ERROR;
		}
	}

	if (identifier_diverged == 0x00) {
		return DALLAS_IDENTIFIER_DONE;
	} else {
		return DALLAS_IDENTIFIER_NO_ERROR;
	}
}

void dallas_write_buffer(uint8_t * buffer, uint8_t buffer_length) {
	uint8_t i;

	for (i = 0x00; i < buffer_length; i++) {
		dallas_write_byte(buffer[i]);
	}
}

void dallas_read_buffer(uint8_t * buffer, uint8_t buffer_length) {
	uint8_t i;

	for (i = 0x00; i < buffer_length; i++) {
		buffer[i] = dallas_read_byte();
	}
}

//Issue dallas command, reset and test bus first - on request.
uint8_t dallas_command(uint8_t command, uint8_t with_reset) {

	if (with_reset) {
		if (!dallas_reset())
			return 0;
	}
	dallas_write_byte(command);
	return 1;
}



/*
 * Utility functions
 */

//Converts Dallas two byte temperature into real like structure
DALLAS_TEMPERATURE getDallasTemp(uint8_t msb, uint8_t lsb) {

	DALLAS_TEMPERATURE temp;
	temp.sign='+';

	uint16_t result;

	//put lsb and msb into int16
	result = msb;
	result = ((result << 8) | lsb);

	//test if temperature is negative then process data
	if (result & 0xF800) {
		//little magic: to convert negative temps you just have to make ...
		result = ~result + 1;
		temp.sign='-';
	}

	//drop 4 lsb bits for integral part
	temp.integer = (result >> 4);
	//use 4 lsb bits for decimal part
	temp.fraction = (result & 0x000F)*PRECISION;

	return temp;
}


void search_bus() {

	uint8_t i;

	//Slaves list
	DALLAS_IDENTIFIER_LIST_t * ids;


	if (dallas_reset()) {

		switch (dallas_search_identifiers()) {
		case 0x00:
			printf("> Bus Test - OK\r\n");
			break;
		case 0x01:
			printf("> Error! Buss error\r\n");
			break;
		case 0x02:
			printf("> Error! More devices then specified\r\n");
			break;
		default:
			printf("> Error! Unknown INIT message\r\n");
			break;
		}

		ids = get_identifier_list();

		//Output IDs found.
		for (i = 0; i < DALLAS_NUM_IDENTIFIER_BITS / 8; i++) {
			printf("%X",ids->identifiers[0].identifier[i]);
			if (i == 7)
				printf("\r\n");
			else
				printf(":");
		}

		printf("> IDs collecting  finished\r\n");

	} else
		printf("BUS Error: No slaves found\r\n");

	return;
}


// converts a dallas temperature type to float for the avr
float DTtof(DALLAS_TEMPERATURE dt)
{
	const int MAX_TEMP_SIZE = 30;
	char str[MAX_TEMP_SIZE];
  	float d = 0.0;

	memset(str, '\0', MAX_TEMP_SIZE);
	sprintf(str, "%d.%d", dt.integer, dt.fraction);
  	char* pEnd;
  	d = (float)strtod (str, &pEnd);
  	if (dt.sign == '-')
  	{
  		d *= -1;
  	}
  	return (d);
}

/*
float CelsiusToFahrenheit(float celsius) 
{
    return (celsius * 9 / 5 + 32);
}
*/

float getTemperatureC(void)
{
	uint8_t chip_scratchpad[9];

		if (dallas_command(SKIP_ROM_COMMAND, 1)) 
		{
			dallas_command(CONVERT_TEMP__COMMAND, 0);
			_delay_ms(500); //wait for conversion result.

			if (dallas_command(SKIP_ROM_COMMAND, 1)) 
			{
				dallas_command(READ_SCRATCHPAD_COMMAND, 0);
				dallas_read_buffer(chip_scratchpad, 9);
				return (DTtof(getDallasTemp(chip_scratchpad[1],chip_scratchpad[0])));

			}
		}
		return -9999.99;
}



