CC = gcc
CFLAGS = -O3 -Wall -fopenmp
LIBS = -lm

all: serial openmp

serial: src/serial_main.c src/filters.c
	$(CC) $(CFLAGS) $^ -o serial_app $(LIBS)

openmp: src/openmp_main.c src/filters.c
    $(CC) $(CFLAGS) $^ -o openmp_app $(LIBS)

clean:
	rm -f serial_app openmp_app output_serial.jpg