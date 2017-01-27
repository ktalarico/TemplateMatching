CC=gcc
CFLAGS=-fopenmp -O3 -march=native -mtune=native -std=c99
LDFLAGS=-lm -fopenmp 

tm: main.o
	gcc -o tm main.o $(LDFLAGS)

clean:
	rm main.o
	rm tm
