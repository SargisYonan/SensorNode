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
BAUDRATE					 = 115200
TARGET=gy30
MCU=atmega2560
DUDE=avrdude
CC=avr-gcc
F_CPU=16000000UL
OPTS=-Wl,-u,vfprintf -lprintf_flt -lm
MAIN = I2C_test

.PHONY: clean

all: $(TARGET).hex


$(MAIN).o: $(MAIN).c
	$(CC) -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(OPTS) -c $^

twimaster.o: ../i2cmaster.h ../twimaster.c
	$(CC) -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(OPTS) -c ../twimaster.c

$(TARGET).elf: $(MAIN).o twimaster.o
	$(CC) -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(OPTS) $^ -o $(TARGET).elf

$(TARGET).lst: $(TARGET).elf
	avr-objdump -h -S $(TARGET).elf > $(TARGET).lst

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex

upload:
ifndef PROGRAMMER_PATH
	$(error "OS not configured in makefile: $(UNAME)")
	exit 1
endif
	${BURNER} -P ${PROGRAMMER_PATH} -p ${AVRDUDE_CONTROLLER} -c ${COMPILER} -e -b ${BAUDRATE} -U flash:w:${TARGET}.hex


clean:
	rm -rf $(TARGET).elf $(TARGET).lst $(TARGET).hex *.o
