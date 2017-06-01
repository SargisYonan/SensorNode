#include "humidity_sensor.h"

#include <avr/pgmspace.h>
#include <util/delay.h>

#define AM2315_TWI_ADDRESS 0x5C // could be 0xB8 or 0x5c according to datasheet
#define AM2315_TWI_ADDRESS_WRITE ((AM2315_TWI_ADDRESS << 1) | I2C_WRITE)
#define AM2315_TWI_ADDRESS_READ ((AM2315_TWI_ADDRESS << 1) | I2C_READ)
#define READREGCODE 0x03
#define BEGINREG 0x00
#define NUMREGTOREAD 4
#define NUMBYTESTOSTORE 6

#include "uart.h"
#include "twimaster.h"

static uint8_t humidity_sensor_count = 0;

// sets an index of the humidity_sensor module array to be the new humidity_sensor's info
// also sets the fields accordingly
// RETURNS:
// the humidity_sensor with fields sest appropriately
// or a default module if too many humidity_sensors already exist
Humidity_Sensor new_humidity_sensor(uint8_t type_num, Humidity_Sensor h) {
  if (humidity_sensor_count >= HUMIDITY_SENSOR_MAX) {
    return h; // remember the key is that it has defaults set
  }
  h.type_num = type_num;
  h.init = &humidity_sensor_init;
  h.read = &humidity_sensor_read;
  h.destroy = &humidity_sensor_destroy;
  humidity_sensor_count++;
  return h;
}

void humidity_sensor_init(Humidity_Sensor h) {
  i2c_init();
  uart_puts_P(PSTR("Humidity_Sensor initialized\r\n"));
  humidity_sensor_read(h);
}

// TODO: double check datasheet, could be faster (smaller delays)
void humidity_sensor_read(Humidity_Sensor h) {
  i2c_start(AM2315_TWI_ADDRESS_WRITE); // Sensor doesn't respond to start signal
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
    uart_puts_P(PSTR("Error reading humidity sensor\r\n"));
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

void humidity_sensor_destroy(Humidity_Sensor h) {
  uart_puts_P(PSTR("Humidity_Sensor cleared\r\n"));
}
