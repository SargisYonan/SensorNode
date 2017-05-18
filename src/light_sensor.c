#include "light_sensor.h"

#include <avr/pgmspace.h>
#include <util/delay.h>

#define AM2315_TWI_ADDRESS 0x5c // could be 0xB8 according to datasheet
#define AM2315_TWI_ADDRESS_WRITE ((AM2315_TWI_ADDRESS << 1) | I2C_WRITE)
#define AM2315_TWI_ADDRESS_READ ((AM2315_TWI_ADDRESS << 1) | I2C_READ)
#define READREGCODE 0x03
#define BEGINREG 0x00
#define NUMREGTOREAD 4
#define NUMBYTESTOSTORE 6

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

void light_sensor_init(Light_Sensor h) {
  i2c_init();

  /* Make sure we're actually connected */
  //uint8_t x = read8(TSL2561_REGISTER_ID); // TODO
  if (!(x & 0x0A))
  {
  //  return false; // TODO: error stuff
  }
  // _tsl2561Initialised = true; // TODO: 

  /* Set default integration time and gain */
  // setIntegrationTime(_tsl2561IntegrationTime); // TODO
  // setGain(_tsl2561Gain); // TODO

  /* Note: by default, the device is in power down mode on bootup */
  //disable(); // TODO

  uart_puts_P(PSTR("Light_Sensor initialized\r\n"));
}

// FIXME should be light sensor
// TODO: double check datasheet, could be faster (smaller delays)
void light_sensor_read(Light_Sensor h) {
  if (i2c_start(AM2315_TWI_ADDRESS_WRITE) != 0) {
    uart_puts_P(PSTR("Couldn't communicate with light sensor\r\n"));
    return;
  }
  _delay_ms(2); // TODO: wait could be shorter?
  i2c_stop(); // should be woke now
  i2c_start(AM2315_TWI_ADDRESS_WRITE); // tell it to generate the temp and hum
  i2c_write(READREGCODE);
  i2c_write(BEGINREG);
  i2c_write(NUMREGTOREAD);
  i2c_stop();
  _delay_ms(10); // TODO: wait could be shorter?
  uint8_t ret[NUMBYTESTOSTORE];
  i2c_start(AM2315_TWI_ADDRESS_READ); // now read the data
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
