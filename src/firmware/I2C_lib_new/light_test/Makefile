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

twimaster.o: i2cmaster.h twimaster.c
	$(CC) -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(OPTS) -c twimaster.c

$(TARGET).elf: $(MAIN).o twimaster.o
	$(CC) -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(OPTS) $^ -o $(TARGET).elf

$(TARGET).lst: $(TARGET).elf
	avr-objdump -h -S $(TARGET).elf > $(TARGET).lst

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex

clean:
	rm -rf $(TARGET).elf $(TARGET).lst $(TARGET).hex *.o
