AVR_GCC_CONTROLLER           = atmega2560
BURNER						 = avrdude
AVRDUDE_CONTROLLER 	         = m2560
CC							 = avr-gcc

################################################################
LIGHT_SENSOR_CSOURCE		= I2C_lib/i2c_master.c
LIGHT_SENSOR_FLAGS			= -DI2C_LIGHT_SENSOR -DDEVICE_TYPE=0x01
LIGHT_SENSOR_OBJECTS 		= i2c_master.o
################################################################
################################################################
TEMPERATURE_SENSOR_CSOURCE		= OneWire/OneWire.c
TEMPERATURE_SENSOR_FLAGS		= -DONE_WIRE_TEMP_SENS -DDEVICE_TYPE=0x02
TEMPERATURE_SENSOR_OBJECTS 		= OneWire.o
################################################################
################################################################
DHT11_SENSOR_CSOURCE		= DHT11/DHT.c
DHT11_SENSOR_FLAGS			= -DDHT11 -DDEVICE_TYPE=0x03
DHT11_SENSOR_OBJECTS 		= DHT.o
################################################################

BASE_CSOURCE				 = main.c UART_LIBRARY/uart.c Sensor_Driver/driver.c
OBJECTS  					 = main.o uart.o driver.o 
EXECUTABLE					 = main
COMPILER_PATH				 = /dev/tty.usbmodem1421
WIN_COMPILER_PATH		 = //./COM3 #windows compiler path assuming it's running on COM3
COMPILER  					 = stk500v2
BAUDRATE					 = 115200
OBJECT_COPY					 = avr-objcopy
CFLAGS						 = -Wall -Werror -Wextra -Wimplicit -std=gnu99
PHASE_ONE					 = -Os -mmcu=${AVR_GCC_CONTROLLER} -DF_CPU=16000000UL ${CFLAGS} -c
PHASE_TWO					 = -mmcu=${AVR_GCC_CONTROLLER} -Wl,-u,vfprintf -lprintf_flt -lm -o${EXECUTABLE}.elf ${OBJECTS}

.PHONY: clean, all
all:
	@echo "INCORRECT USAGE"
	@echo "SELECT A CORRECT TARGET"

clean:
	rm -f ${OBJECTS} ${EXECUTABLE}.hex ${EXECUTABLE}.elf

light:
	${CC} ${PHASE_ONE} ${LIGHT_SENSOR_FLAGS} ${BASE_CSOURCE} ${LIGHT_SENSOR_CSOURCE}
	${CC} ${PHASE_TWO} ${LIGHT_SENSOR_OBJECTS}
	${OBJECT_COPY} -j .text -j .data -O ihex ${EXECUTABLE}.elf ${EXECUTABLE}.hex
	${BURNER} -P ${COMPILER_PATH} -p ${AVRDUDE_CONTROLLER} -c ${COMPILER} -e -b ${BAUDRATE} -U flash:w:${EXECUTABLE}.hex 
	rm -f ${OBJECTS} ${EXECUTABLE}.elf

#same as light but uses Windows compiler path instead
wlight:
	${CC} ${PHASE_ONE} ${LIGHT_SENSOR_FLAGS} ${BASE_CSOURCE} ${LIGHT_SENSOR_CSOURCE}
	${CC} ${PHASE_TWO} ${LIGHT_SENSOR_OBJECTS}
	${OBJECT_COPY} -j .text -j .data -O ihex ${EXECUTABLE}.elf ${EXECUTABLE}.hex
	${BURNER} -P ${WIN_COMPILER_PATH} -p ${AVRDUDE_CONTROLLER} -c ${COMPILER} -e -b ${BAUDRATE} -U flash:w:${EXECUTABLE}.hex 
	rm -f ${OBJECTS} ${EXECUTABLE}.elf

temp:
	${CC} ${PHASE_ONE} ${TEMPERATURE_SENSOR_FLAGS} ${BASE_CSOURCE} ${TEMPERATURE_SENSOR_CSOURCE}
	${CC} ${PHASE_TWO} ${TEMPERATURE_SENSOR_OBJECTS}
	${OBJECT_COPY} -j .text -j .data -O ihex ${EXECUTABLE}.elf ${EXECUTABLE}.hex
	${BURNER} -P ${COMPILER_PATH} -p ${AVRDUDE_CONTROLLER} -c ${COMPILER} -e -b ${BAUDRATE} -U flash:w:${EXECUTABLE}.hex 
	rm -f ${OBJECTS} ${EXECUTABLE}.elf

#same as temp but uses windows compiler path instead
wtemp:
	${CC} ${PHASE_ONE} ${TEMPERATURE_SENSOR_FLAGS} ${BASE_CSOURCE} ${TEMPERATURE_SENSOR_CSOURCE}
	${CC} ${PHASE_TWO} ${TEMPERATURE_SENSOR_OBJECTS}
	${OBJECT_COPY} -j .text -j .data -O ihex ${EXECUTABLE}.elf ${EXECUTABLE}.hex
	${BURNER} -P ${WIN_COMPILER_PATH} -p ${AVRDUDE_CONTROLLER} -c ${COMPILER} -e -b ${BAUDRATE} -U flash:w:${EXECUTABLE}.hex 
	rm -f ${OBJECTS} ${EXECUTABLE}.elf

dht:
	${CC} ${PHASE_ONE} ${DHT11_SENSOR_FLAGS} ${BASE_CSOURCE} ${DHT11_SENSOR_CSOURCE}
	${CC} ${PHASE_TWO} ${DHT11_SENSOR_OBJECTS}
	${OBJECT_COPY} -j .text -j .data -O ihex ${EXECUTABLE}.elf ${EXECUTABLE}.hex
	${BURNER} -P ${COMPILER_PATH} -p ${AVRDUDE_CONTROLLER} -c ${COMPILER} -e -b ${BAUDRATE} -U flash:w:${EXECUTABLE}.hex 
	rm -f ${OBJECTS} ${EXECUTABLE}.elf

#same as dht but uses windows compiler path instead
wdht:
	${CC} ${PHASE_ONE} ${DHT11_SENSOR_FLAGS} ${BASE_CSOURCE} ${DHT11_SENSOR_CSOURCE}
	${CC} ${PHASE_TWO} ${DHT11_SENSOR_OBJECTS}
	${OBJECT_COPY} -j .text -j .data -O ihex ${EXECUTABLE}.elf ${EXECUTABLE}.hex
	${BURNER} -P ${WIN_COMPILER_PATH} -p ${AVRDUDE_CONTROLLER} -c ${COMPILER} -e -b ${BAUDRATE} -U flash:w:${EXECUTABLE}.hex 
	rm -f ${OBJECTS} ${EXECUTABLE}.elf

