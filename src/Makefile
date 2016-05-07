PROGRAMMER=avrdude
LINPORT=/dev/ttyACM0
MACPORT=/dev/tty.usbmodem1421
WINPORT=COM3
CC=avr-gcc
CPP=avr-g++
TARGET=main
OBJCOPY=avr-objcopy
RM=rm -f
F_CPU=16000000UL
CDEFS=-DF_CPU=$(F_CPU)
CDEFS+=-DUART_TX_BUFFER_SIZE=16
CDEFS+=-DUART_RX_BUFFER_SIZE=16
CDEFS+=-DNORADIO=1
#CDEFS+=-DDHT_SENSOR
CDEFS+=-DLIGHT_SENSOR
ifdef DEBUG
	CDEFS+=-DDEBUG
endif
OPT=s
CSTANDARD=-std=gnu99
MCU=atmega2560


CFLAGS=-Wall 
CFLAGS+=-Wextra
CFLAGS+=-Wimplicit
CFLAGS+=-Wstrict-prototypes
CFLAGS+=-Wundef
CFLAGS+=-Wunreachable-code
CFLAGS+=-Wsign-compare
CFLAGS+=$(CSTANDARD)
CFLAGS+=-Wa,-adhlns=$(<:%.c=%.lst)
CFLAGS+=-lm
LDFLAGS=-Wl,-Map=$(TARGET).map,--cref
LDFLAGS+=-Wl,-u,vfprintf -lprintf_flt -lm

CFLAGS+=-O$(OPT)
LDFLAGS+=-O$(OPT)

######## OS Detection #########
ifeq ($(OS), Windows_NT)
	PROGRAMMER_PATH = $(WINPORT)
else
	UNAME = $(shell uname -s)
	ifeq ($(UNAME),Linux)
		PROGRAMMER_PATH=$(LINPORT)
	else
		ifeq ($(UNAME),Darwin)
			PROGRAMMER_PATH=$(MACPORT)
		endif
	endif
endif

####### Targets ########
.PHONY: clean, upload

all: $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex

$(TARGET).elf: main.o I2C_lib.o twimaster.o uart.o parser.o
	$(CC) -mmcu=$(MCU) $(LDFLAGS) main.o I2C_lib.o twimaster.o uart.o parser.o -o $(TARGET).elf

dht.o: dht.c dht.h
	$(CC) -mmcu=$(MCU) $(CFLAGS) $(CDEFS) -c dht.c

main.o: main.c dht.h uart_macros.h uart.h
	$(CC) -mmcu=$(MCU) $(CFLAGS) $(CDEFS) -c main.c

uart.o: uart.c uart.h
	$(CC) -mmcu=$(MCU) $(CFLAGS) $(CDEFS) -c uart.c

parser.o: parser.c parser.h uart.h uart_macros.h
	$(CC) -mmcu=$(MCU) $(CFLAGS) $(CDEFS) -c parser.c

twimaster.o: twimaster.c i2cmaster.h
	$(CC) -mmcu=$(MCU) $(CFLAGS) $(CDEFS) -c twimaster.c

I2C_lib.o: I2C_lib.c I2C_lib.h
	$(CC) -mmcu=$(MCU) $(CFLAGS) $(CDEFS) -c I2C_lib.c

clean:
	$(RM) $(TARGET).elf $(TARGET).hex *.o $(TARGET).map *.lst

upload:
ifndef PROGRAMMER_PATH
	$(error "OS not configured in makefile: $(UNAME)")
	exit 1
endif
	$(PROGRAMMER) -P $(PROGRAMMER_PATH) -p $(MCU) -c stk500v2 -e -b 115200 -U flash:w:$(TARGET).hex
