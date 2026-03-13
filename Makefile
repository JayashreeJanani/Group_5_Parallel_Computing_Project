CC = gcc
CFLAGS = -O3 -Wall
LIBS = -lm

all: serial

serial: src/serial_main.c src/filters.c
	$(CC) $(CFLAGS) $^ -o serial_app $(LIBS)

clean:
	rm -f serial_app output_serial.jpg