CC=gcc -Wall

all: build test clean

build: main.c worker1.c worker2.c worker3.c
	$(CC) -o main main.c -lpthread -lrt
	$(CC) -o worker1 worker1.c -lpthread -lrt
	$(CC) -o worker2 worker2.c -lpthread -lrt
	$(CC) -o worker3 worker3.c -lpthread -lrt

test: main worker1 worker2 worker3
	./main

clean:
	rm -f main worker1 worker2 worker3
