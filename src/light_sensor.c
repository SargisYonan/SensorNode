#include "light_sensor.h"

#include <avr/pgmspace.h>
#include <util/delay.h>

#define TSL2561_FLOATING_TWI_ADDRESS 0x39 // could be 0xB8 according to datasheet
#define TSL2561_REGISTER_ID 0x0A
#define TSL2561_TWI_ADDRESS_WRITE ((TSL2561_FLOATING_TWI_ADDRESS << 1) | I2C_WRITE)
#define TSL2561_TWI_ADDRESS_READ ((TSL2561_FLOATING_TWI_ADDRESS << 1) | I2C_READ)
#define TSL2561_COMMAND_BIT 0x80
#define TSL2561_REGISTER_CONTROL 0x00
#define TSL2561_CONTROL_POWERON 0x03
#define TSL2561_CONTROL_POWEROFF 0x00
#define TSL2561_INTEGRATIONTIME_13MS 0x00    // 13.7ms
#define TSL2561_INTEGRATIONTIME_101MS 0x01    // 101ms
#define TSL2561_INTEGRATIONTIME_402MS 0x02     // 402ms
#define TSL2561_INTEGRATION_TIME TSL2561_INTEGRATIONTIME_402MS // set
                                        // integration tim to 402 ms
#define TSL2561_GAIN 0x00 // 0x00 for no gain, 0x10 for 16x gain

#include "uart.h"
#include "twimaster.h"

static uint8_t light_sensor_count = 0;
static uint8_t light_sensor_type_num = -1; // needs to be set on first creation of Light_Sensor

// sets an index of the light_sensor module array to be the new light_sensor's info
// also sets the fields accordingly
// RETURNS:
// the light_sensor with fields sest appropriately
// or a default module if too many light_sensors already exist
Light_Sensor new_light_sensor(uint8_t type_num, Light_Sensor h) {
  if (light_sensor_count >= LIGHT_SENSOR_MAX) {
    return h; // remember the key is that it has defaults set
  }
  if (light_sensor_count == 0) {
    light_sensor_type_num = type_num;
  }
  h.type_num = light_sensor_type_num;
  h.init = &light_sensor_init;
  h.read = &light_sensor_read;
  h.destroy = &light_sensor_destroy;
  light_sensor_count++;
  return h;
}

// should be good
void light_sensor_init(Light_Sensor h) {
  i2c_init();

  /* Make sure we're actually connected */
  i2c_start(TSL2561_TWI_ADDRESS_READ);
  uint8_t x = i2c_read(0);
  i2c_stop();
  if (!(x & 0x0A))
  {
    uart_puts_P(PSTR("Couldn't communicate with light sensor\r\n"));
    return;
  }

  i2c_start(TSL2561_TWI_ADDRESS_WRITE);

  i2c_write(TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL,
      TSL2561_CONTROL_POWERON); // enable the light sensor

  i2c_write(TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING,
      TSL2561_INTEGRATION_TIME | TSL2561_GAIN); // set the integration time and gain

  i2c_write(TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL,
      TSL2561_CONTROL_POWERON); // disable the light sensor

  i2c_stop();

  uart_puts_P(PSTR("light sensor initialized\r\n"));
}

// FIXME should be light sensor
// TODO: double check datasheet, could be faster (smaller delays)
void light_sensor_read(Light_Sensor h) {
  if (i2c_start(TSL2561_TWI_ADDRESS_WRITE) != 0) {
    uart_puts_P(PSTR("Couldn't communicate with light sensor\r\n"));
    return;
  }
  _delay_ms(2); // TODO: wait could be shorter?
  i2c_stop(); // should be woke now
  i2c_start(TSL2561_TWI_ADDRESS_WRITE); // tell it to generate the temp and hum
  i2c_write(READREGCODE);
  i2c_write(BEGINREG);
  i2c_write(NUMREGTOREAD);
  i2c_stop();
  _delay_ms(10); // TODO: wait could be shorter?
  uint8_t ret[NUMBYTESTOSTORE];
  i2c_start(TSL2561_TWI_ADDRESS_READ); // now read the data
  for (int i = 0; i < NUMBYTESTOSTORE - 1; i++) ret[i] = i2c_read(1);
  ret[NUMBYTESTOSTORE - 1] = i2c_read(0); // this reads and sends a CRC signal
  i2c_stop();
  if (ret[0] != READREGCODE || ret[1] != NUMREGTOREAD) {
    uart_puts_P(PSTR("Error reading light sensor\r\n"));
    return;
  }
  // note that both these values are 10 times what they should be but
  // representing with floating point is a terrible idea so we manipulate the
  // print statement instead
  uint16_t hum = ((ret[2] << 8) | ret[3]);
  uint16_t temp = ((ret[4] << 8) | ret[5]); // NOTE that leading bit is sign
  uart_printf("Humidity is %d.%d %%RH and Temperature is %s%d.%d C\r\n",
      hum / 10, hum % 10, // hum / 10 first, then last digit goes after '.'
      ret[4] & 0x80 ? "-" : "", // handle the sign (if leading is set, then neg)
      (temp & 0x7f) / 10, temp % 10); // same as humidity except don't count b15
}

// FIXME should be light sensor
void light_sensor_destroy(Light_Sensor h) {
  uart_puts_P(PSTR("Light_Sensor cleared\r\n"));
}
