#include "light_sensor.h"

#include <avr/pgmspace.h>
#include <util/delay.h>

#define TSL2591_TWI_ADDRESS 0x29 // there is only one available address
#define TSL2591_TWI_ADDRESS_WRITE ((TSL2591_TWI_ADDRESS << 1) | I2C_WRITE)
#define TSL2591_TWI_ADDRESS_READ ((TSL2591_TWI_ADDRESS << 1) | I2C_READ)

#define TSL2591_CHOSEN_GAIN TSL2591_GAIN_MED
#define TSL2591_CHOSEN_INTEGRATION TSL2591_INTEGRATIONTIME_100MS

// FROM ADAFRUIT TSL2591 LIBRARY //

#define TSL2591_VISIBLE           (2)       // channel 0 - channel 1
#define TSL2591_INFRARED          (1)       // channel 1
#define TSL2591_FULLSPECTRUM      (0)       // channel 0

#define TSL2591_READBIT           (0x01)

#define TSL2591_COMMAND_BIT       (0xA0)    // 1010 0000: bits 7 and 5 for 'command normal'
#define TSL2591_CLEAR_INT         (0xE7)
#define TSL2591_TEST_INT          (0xE4)
#define TSL2591_WORD_BIT          (0x20)    // 1 = read/write word (rather than byte)
#define TSL2591_BLOCK_BIT         (0x10)    // 1 = using block read/write

#define TSL2591_ENABLE_POWEROFF   (0x00)
#define TSL2591_ENABLE_POWERON    (0x01)
#define TSL2591_ENABLE_AEN        (0x02)    // ALS Enable. This field activates ALS function. Writing a one activates the ALS. Writing a zero disables the ALS.
#define TSL2591_ENABLE_AIEN       (0x10)    // ALS Interrupt Enable. When asserted permits ALS interrupts to be generated, subject to the persist filter.
#define TSL2591_ENABLE_NPIEN      (0x80)    // No Persist Interrupt Enable. When asserted NP Threshold conditions will generate an interrupt, bypassing the persist filter

#define TSL2591_LUX_DF            (408)

// Note that these three are 100 times what they should be to avoid floating
// point calculations. Take care in your calculations to accomodate this
#define TSL2591_LUX_COEFB         (164)  // CH0 coefficient
#define TSL2591_LUX_COEFC         (59)  // CH1 coefficient A
#define TSL2591_LUX_COEFD         (86)  // CH2 coefficient B

// END ADAFRUIT LIBRARY CODE //

#include "uart.h"
#include "twimaster.h"

static uint8_t light_sensor_count = 0;

// FROM ADAFRUIT TSL2591 LIBRARY //

enum
{
  TSL2591_REGISTER_ENABLE             = 0x00,
  TSL2591_REGISTER_CONTROL            = 0x01,
  TSL2591_REGISTER_THRESHOLD_AILTL    = 0x04, // ALS low threshold lower byte
  TSL2591_REGISTER_THRESHOLD_AILTH    = 0x05, // ALS low threshold upper byte
  TSL2591_REGISTER_THRESHOLD_AIHTL    = 0x06, // ALS high threshold lower byte
  TSL2591_REGISTER_THRESHOLD_AIHTH    = 0x07, // ALS high threshold upper byte
  TSL2591_REGISTER_THRESHOLD_NPAILTL  = 0x08, // No Persist ALS low threshold lower byte
  TSL2591_REGISTER_THRESHOLD_NPAILTH  = 0x09, // etc
  TSL2591_REGISTER_THRESHOLD_NPAIHTL  = 0x0A,
  TSL2591_REGISTER_THRESHOLD_NPAIHTH  = 0x0B,
  TSL2591_REGISTER_PERSIST_FILTER     = 0x0C,
  TSL2591_REGISTER_PACKAGE_PID        = 0x11,
  TSL2591_REGISTER_DEVICE_ID          = 0x12,
  TSL2591_REGISTER_DEVICE_STATUS      = 0x13,
  TSL2591_REGISTER_CHAN0_LOW          = 0x14,
  TSL2591_REGISTER_CHAN0_HIGH         = 0x15,
  TSL2591_REGISTER_CHAN1_LOW          = 0x16,
  TSL2591_REGISTER_CHAN1_HIGH         = 0x17
};

typedef enum
{
  TSL2591_INTEGRATIONTIME_100MS     = 0x00,
  TSL2591_INTEGRATIONTIME_200MS     = 0x01,
  TSL2591_INTEGRATIONTIME_300MS     = 0x02,
  TSL2591_INTEGRATIONTIME_400MS     = 0x03,
  TSL2591_INTEGRATIONTIME_500MS     = 0x04,
  TSL2591_INTEGRATIONTIME_600MS     = 0x05,
}
tsl2591IntegrationTime_t;

typedef enum
{
  //  bit 7:4: 0
  TSL2591_PERSIST_EVERY             = 0x00, // Every ALS cycle generates an interrupt
  TSL2591_PERSIST_ANY               = 0x01, // Any value outside of threshold range
  TSL2591_PERSIST_2                 = 0x02, // 2 consecutive values out of range
  TSL2591_PERSIST_3                 = 0x03, // 3 consecutive values out of range
  TSL2591_PERSIST_5                 = 0x04, // 5 consecutive values out of range
  TSL2591_PERSIST_10                = 0x05, // 10 consecutive values out of range
  TSL2591_PERSIST_15                = 0x06, // 15 consecutive values out of range
  TSL2591_PERSIST_20                = 0x07, // 20 consecutive values out of range
  TSL2591_PERSIST_25                = 0x08, // 25 consecutive values out of range
  TSL2591_PERSIST_30                = 0x09, // 30 consecutive values out of range
  TSL2591_PERSIST_35                = 0x0A, // 35 consecutive values out of range
  TSL2591_PERSIST_40                = 0x0B, // 40 consecutive values out of range
  TSL2591_PERSIST_45                = 0x0C, // 45 consecutive values out of range
  TSL2591_PERSIST_50                = 0x0D, // 50 consecutive values out of range
  TSL2591_PERSIST_55                = 0x0E, // 55 consecutive values out of range
  TSL2591_PERSIST_60                = 0x0F, // 60 consecutive values out of range
}
tsl2591Persist_t;

typedef enum
{
  TSL2591_GAIN_LOW                  = 0x00,    // low gain (1x)
  TSL2591_GAIN_MED                  = 0x10,    // medium gain (25x)
  TSL2591_GAIN_HIGH                 = 0x20,    // medium gain (428x)
  TSL2591_GAIN_MAX                  = 0x30,    // max gain (9876x)
}
tsl2591Gain_t;

// END ADAFRUIT LIBRARY CODE //

// private function that attempts to read device id (checks if it is connected)
// returns 1 if successful, 0 otherwise
uint8_t TSL2591_check_connectivity(void) {

  // request the data
  if (i2c_start(TSL2591_TWI_ADDRESS_WRITE)) return 0;
  i2c_write(TSL2591_COMMAND_BIT | TSL2591_REGISTER_DEVICE_ID);
  i2c_stop();

  //uart_puts_P(PSTR("Checking connectivity finished 1\r\n"));
  // then read the data
  if (i2c_start(TSL2591_TWI_ADDRESS_READ)) return 0;
  uint8_t id = i2c_read(0);
  i2c_stop();

  //uart_puts_P(PSTR("Checking connectivity finished 2\r\n"));
  return (id == 0x50 ? 1 : 0); // id should be returned as 0x50 normally
}

// private function that enables the light sensor
// returns 1 if successful, 0 otherwise
uint8_t TSL2591_enable(void) {
  if (i2c_start(TSL2591_TWI_ADDRESS_WRITE)) return 0;

  // enable device and ambient light sensor
  i2c_write(TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE);
  i2c_write(TSL2591_ENABLE_POWERON | TSL2591_ENABLE_AEN | TSL2591_ENABLE_AIEN |
      TSL2591_ENABLE_NPIEN);

  i2c_stop();
  return 1;
}

// private function that disables the light sensor
// returns 1 if successful, 0 otherwise
uint8_t TSL2591_disable(void) {
  if (i2c_start(TSL2591_TWI_ADDRESS_WRITE)) return 0;

  // disable device
  i2c_write(TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE);
  i2c_write(TSL2591_ENABLE_POWEROFF);

  i2c_stop();
  return 1;
}

// private function that sets the gain and integration time of the sensor
uint8_t TSL2591_set_gain_integration(uint8_t gain, uint8_t integration) {
  TSL2591_enable();

  if (i2c_start(TSL2591_TWI_ADDRESS_WRITE)) return 0;

  // set the gain and integration time of the device
  i2c_write(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONTROL);
  i2c_write(gain | integration);

  i2c_stop();

  TSL2591_disable();
  return 1;
}

// sets an index of the light_sensor module array to be the new light_sensor's info
// also sets the fields accordingly
// RETURNS:
// the light_sensor with fields sest appropriately
// or a default module if too many light_sensors already exist
Light_Sensor new_light_sensor(uint8_t type_num, Light_Sensor ls) {
  if (light_sensor_count >= LIGHT_SENSOR_MAX) {
    return ls; // remember the key is that it has defaults set
  }
  ls.type_num = type_num;
  ls.init = &light_sensor_init;
  ls.read = &light_sensor_read;
  ls.destroy = &light_sensor_destroy;
  //light_sensor_count++;
  return ls;
}

// should be good
void light_sensor_init(Light_Sensor ls) {
  if (ls.write) {} // stop complaining
  i2c_init();

  // Make sure we're actually connected and operating as expected
  if (!TSL2591_check_connectivity())
  {
    uart_puts_P(PSTR("Couldn't communicate with light sensor\r\n"));
    return;
  }

  // set to default values by adafruit library (note this also disables device)
  if (!TSL2591_set_gain_integration(
      TSL2591_CHOSEN_GAIN, TSL2591_CHOSEN_INTEGRATION))
  {
    uart_puts_P(PSTR("Couldn't communicate with light sensor\r\n"));
    return;
  }
  uart_puts_P(PSTR("light sensor initialized\r\n"));
}

// Adafruit admits in their source code that this algorithm might be out of date
void light_sensor_read(Light_Sensor ls, char *read_data, uint16_t max_bytes) {
  if (ls.write) {} // stop complaining
  if (!TSL2591_check_connectivity()) {
    uart_puts_P(PSTR("Couldn't communicate with light sensor\r\n"));
    return;
  }
  TSL2591_enable();
  for (uint8_t i = 0; i <= TSL2591_CHOSEN_INTEGRATION; i++)
    _delay_ms(120); // in 100ms steps except that we give 20 ms extra time each

  // request data for channel 1
  if (i2c_start(TSL2591_TWI_ADDRESS_WRITE)) {
    uart_puts_P(PSTR("Couldn't communicate with light sensor\r\n"));
    return;
  }
  i2c_write(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN1_LOW);
  i2c_stop();

  if (i2c_start(TSL2591_TWI_ADDRESS_READ)) {
    uart_puts_P(PSTR("Couldn't communicate with light sensor\r\n"));
    return;
  }
  uint16_t chan1 = i2c_read(1);
  chan1 = (i2c_read(0) << 8); // bytes sent in reverse order
  i2c_stop();

  // request data for channel 0
  if (i2c_start(TSL2591_TWI_ADDRESS_WRITE)) {
    uart_puts_P(PSTR("Couldn't communicate with light sensor\r\n"));
    return;
  }
  i2c_write(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW);
  i2c_stop();

  if (i2c_start(TSL2591_TWI_ADDRESS_READ)) {
    uart_puts_P(PSTR("Couldn't communicate with light sensor\r\n"));
    return;
  }
  uint16_t chan0 = i2c_read(1);
  chan0 = (i2c_read(0) << 8); // bytes sent in reverse order
  i2c_stop();

  TSL2591_disable();

  // Note that the following was modified for better readability and
  // optimization

  // BEGIN ARDUINO LIBRARY CODE //

  uint16_t    atime, again;
  uint32_t    cpl, lux1, lux2, lux;

  // Check for overflow conditions first
  if ((chan0 == 0xFFFF) | (chan1 == 0xFFFF))
  {
    // Signal an overflow
    uart_puts_P(PSTR("Overflow in reading light sensor data\r\n"));
    return;
  }

  // Note: This algorithm is based on preliminary coefficients
  // provided by AMS and may need to be updated in the future

  // convert to actual value in ms
  atime = (TSL2591_CHOSEN_INTEGRATION + 1) * 100;

  switch (TSL2591_CHOSEN_GAIN)
  {
    case TSL2591_GAIN_LOW :
      again = 1.0;
      break;
    case TSL2591_GAIN_MED :
      again = 25.0;
      break;
    case TSL2591_GAIN_HIGH :
      again = 428.0;
      break;
    case TSL2591_GAIN_MAX :
      again = 9876.0;
      break;
    default:
      again = 1.0;
      break;
  }

  // NOTE: the LUX COEFs are 100 time what they used to be (look at defines)

  cpl = (atime * again) / TSL2591_LUX_DF;

  lux1 = (((chan0 * 100) - (TSL2591_LUX_COEFB * chan1)) / (100 * cpl));
  lux2 = ((TSL2591_LUX_COEFC * chan0) - (TSL2591_LUX_COEFD * chan1)) /
    (cpl * 100);
  lux = lux1 > lux2 ? lux1 : lux2;

  // Signal I2C had no errors
  uart_printf("Light reading is %lu lx\r\n", lux);
  snprintf(read_data, max_bytes, "%lu lx\r\n", lux);

  // END ARDUINO LIBRARY CODE //
}

void light_sensor_destroy(Light_Sensor ls) {
  if (ls.write) {} // stop complaining
  TSL2591_disable(); // should already be disabled but lets be assured
  uart_puts_P(PSTR("Light_Sensor cleared\r\n"));
}
