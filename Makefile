PROGRAMMER=avrdude
PROGRAMMER_PORT=/dev/ttyACM0
CC=avr-gcc
CPP=avr-g++
TARGET=main
OBJCOPY=avr-objcopy
RM=rm -f
F_CPU=16000000UL
CDEFS=-DF_CPU=$(F_CPU)
CDEFS+=-DUART_TX_BUFFER_SIZE=16
CDEFS+=-DUART_RX_BUFFER_SIZE=16
CDEFS+=-DDHT_SENSOR
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

.PHONY: clean, upload

all: $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex

$(TARGET).elf: main.o dht.o uart.o parser.o
	$(CC) -mmcu=$(MCU) $(LDFLAGS) main.o dht.o uart.o parser.o -o $(TARGET).elf

dht.o: dht.c dht.h
	$(CC) -mmcu=$(MCU) $(CFLAGS) $(CDEFS) -c dht.c

main.o: main.c dht.h uart_macros.h uart.h
	$(CC) -mmcu=$(MCU) $(CFLAGS) $(CDEFS) -c main.c

uart.o: uart.c uart.h
	$(CC) -mmcu=$(MCU) $(CFLAGS) $(CDEFS) -c uart.c

parser.o: parser.c parser.h uart.h
	$(CC) -mmcu=$(MCU) $(CFLAGS) $(CDEFS) -c parser.c

clean:
	$(RM) $(TARGET).elf $(TARGET).hex *.o $(TARGET).map *.lst

upload:
	$(PROGRAMMER) -P $(PROGRAMMER_PORT) -p $(MCU) -c stk500v2 -e -b 115200 -U flash:w:$(TARGET).hex
