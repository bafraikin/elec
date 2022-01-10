MCU=atmega328p
F_CPU=16000000
CC=avr-gcc
OBJCOPY=avr-objcopy
CFLAGS=-mmcu=${MCU} -DF_CPU=${F_CPU}
TARGET=main
SRCS=main.c



all: hex flash

hex:
	${CC} ${CFLAGS} -o ${TARGET}.bin ${SRCS}
	${OBJCOPY} -O ihex ${TARGET}.bin ${TARGET}.hex

flash:
	avrdude -p ${MCU} -c arduino -U flash:w:${TARGET}.hex:i -F -P /dev/ttyUSB0


clean: 
	rm main.hex main.bin
