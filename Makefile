#grupo 57
#Carolina Carreira 87641
#Miguel Barros 87691

CFLAGS= -g -Wall -pedantic
CC=gcc


heatSim: main.o matrix2d.o mplib3.o leQueue.o leQueue.h mplib3.h
	$(CC) $(CFLAGS) -pthread -o heatSim main.o matrix2d.o mplib3.o leQueue.o

matrix2d.o: matrix2d.c matrix2d.h
	$(CC) $(CFLAGS) -c matrix2d.c

leQueue.o: leQueue.c leQueue.h
	$(CC) $(CFLAGS) -c leQueue.c

mplib3.o: mplib3.c mplib3.h leQueue.h
	$(CC) $(CFLAGS) -c mplib3.c

clean:
	rm -f *.o heatSim

zip:
	zip heatSim_p2_solucao.zip main.c matrix2d.c matrix2d.h leQueue.c leQueue.h mplib3.c mplib3.h main.c Makefile

run:
	./heatSim 2048 10.1 10.7 6.0 3.5 10 2048 2048
