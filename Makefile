# Makefile, versao 2
# Sistemas Operativos, DEI/IST/ULisboa 2017-18

CFLAGS = -g -Wall -pedantic -pthread
CC = gcc

all: heatSim

heatSim: main.o matrix2d.o
	$(CC) $(CFLAGS)  -o heatSim main.o matrix2d.o

main.o : main.c
	$(CC) $(CFLAGS) -c main.c

matrix2d.o: matrix2d.c matrix2d.h
	$(CC) $(CFLAGS) -c matrix2d.c


clean:
	rm -f *.o heatSim heatSim

zip:
	zip heatSim.zip main.c matrix2d.c matrix2d.h Makefile

run:
	./heatSim 10 10.0 10.0 5.0 5.0 30 5 50
