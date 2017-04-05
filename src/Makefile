PROGRAMMER=avrdude
LINPORT=/dev/ttyACM0
LINPORT=/dev/ttyS0 #working on vbox
MACPORT=/dev/tty.usbmodem1421
WINPORT=COM3
CC=avr-gcc
CPP=avr-g++
TARGET=main
OBJCOPY=avr-objcopy
RM=rm -f
F_CPU=16000000UL
CDEFS=-DF_CPU=$(F_CPU)
# TODO: is this being used anymore?
ifdef NORADIO
	CDEFS+=-DNORADIO
else
	ifdef DEBUG
		CDEFS+=-DDEBUG
	endif
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

MODULES = main parser uart module actuator #OneWire
OBJECTS = $(MODULES:%=%.o)

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

$(TARGET).elf: $(OBJECTS)
	$(CC) -mmcu=$(MCU) $(LDFLAGS) $^ -o $(TARGET).elf

%.o: %.c
	$(CC) -mmcu=$(MCU) $(CFLAGS) $(CDEFS) -c $<

clean:
	$(RM) $(TARGET).elf $(TARGET).hex *.o $(TARGET).map *.lst

upload:
ifndef PROGRAMMER_PATH
	$(error "OS not configured in makefile: $(UNAME)")
	exit 1
endif
	$(PROGRAMMER) -P $(PROGRAMMER_PATH) -p $(MCU) -c stk500v2 -b 115200 -U flash:w:$(TARGET).hex -D
