CC = gcc
MPI_CC = mpicc
CFLAGS = -O3 -Wall -fopenmp
LIBS = -lm

all: serial openmp mpi

serial: src/serial_main.c src/filters.c
	$(CC) $(CFLAGS) $^ -o serial_app $(LIBS)

openmp: src/openmp_main.c src/filters.c
	$(CC) $(CFLAGS) $^ -o openmp_app $(LIBS)

mpi: src/mpi_main.c src/filters.c
	$(MPI_CC) $(CFLAGS) $^ -o mpi_app $(LIBS)

resize: src/resize_main.c
	$(CC) $(CFLAGS) $^ -o resize_app $(LIBS)

clean:
	rm -f serial_app openmp_app mpi_app
	./scripts/reset_outputs.sh