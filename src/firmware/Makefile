AVR_GCC_CONTROLLER           = atmega2560
BURNER						 = avrdude
AVRDUDE_CONTROLLER 	         = m2560
CC							 = avr-gcc
WINPORT = COM3
MACPORT = /dev/tty.usbmodem1421
LINPORT = /dev/ttyUSB0
###### OS Detection ######
ifeq ($(OS), Windows_NT)
	PROGRAMMER_PATH = $(WINPORT)
else
	UNAME = $(shell uname -s)
	ifeq ($(UNAME),Linux)
		PROGRAMMER_PATH = $(LINPORT)
	endif
	ifeq ($(UNAME),Darwin)
		PROGRAMMER_PATH = $(MACPORT)
	endif
endif


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
COMPILER  					 = stk500v2
BAUDRATE					 = 115200
OBJECT_COPY					 = avr-objcopy
CFLAGS						 = -Wall -Werror -Wextra -Wimplicit -std=gnu99
#Flags below
# -Os for our delay library optimizations need to be turned on
# -Wl,-u,vfprintf -lprintf_flt options to configure stdio for our purposes
#  printf_flt is a printf that supports floating point values see
#  http://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html#gaa3b98c0d17b35642c0f3e4649092b9f1
# for other 'optimization options' see
# http://www.nongnu.org/avr-libc/user-manual/using_tools.html#using_sel_gcc_opts
#
PHASE_ONE					 = -Os -mmcu=${AVR_GCC_CONTROLLER} -DF_CPU=16000000UL ${CFLAGS} -c
PHASE_TWO					 = -mmcu=${AVR_GCC_CONTROLLER} -Wl,-u,vfprintf -lprintf_flt -lm -o${EXECUTABLE}.elf ${OBJECTS}

.PHONY: clean, all, upload
all:
	@echo "INCORRECT USAGE"
	@echo "SELECT A CORRECT TARGET"

clean:
	rm -f ${OBJECTS} ${LIGHT_SENSOR_OBJECTS} ${TEMPERATURE_SENSOR_OBJECTS}\
		${DHT11_SENSOR_OBJECTS} ${EXECUTABLE}.hex ${EXECUTABLE}.elf

upload:
ifndef PROGRAMMER_PATH
	$(error "OS not configured in makefile: $(UNAME)")
	exit 1
endif
	${BURNER} -P ${PROGRAMMER_PATH} -p ${AVRDUDE_CONTROLLER} -c ${COMPILER} -e -b ${BAUDRATE} -U flash:w:${EXECUTABLE}.hex

light:
	${CC} ${PHASE_ONE} ${LIGHT_SENSOR_FLAGS} ${BASE_CSOURCE} ${LIGHT_SENSOR_CSOURCE}
	${CC} ${PHASE_TWO} ${LIGHT_SENSOR_OBJECTS}
	${OBJECT_COPY} -j .text -j .data -O ihex ${EXECUTABLE}.elf ${EXECUTABLE}.hex

temp:
	${CC} ${PHASE_ONE} ${TEMPERATURE_SENSOR_FLAGS} ${BASE_CSOURCE} ${TEMPERATURE_SENSOR_CSOURCE}
	${CC} ${PHASE_TWO} ${TEMPERATURE_SENSOR_OBJECTS}
	${OBJECT_COPY} -j .text -j .data -O ihex ${EXECUTABLE}.elf ${EXECUTABLE}.hex

dht:
	${CC} ${PHASE_ONE} ${DHT11_SENSOR_FLAGS} ${BASE_CSOURCE} ${DHT11_SENSOR_CSOURCE}
	${CC} ${PHASE_TWO} ${DHT11_SENSOR_OBJECTS}
	${OBJECT_COPY} -j .text -j .data -O ihex ${EXECUTABLE}.elf ${EXECUTABLE}.hex
