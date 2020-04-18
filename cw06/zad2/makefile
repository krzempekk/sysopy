CC=gcc -Wall

all: build test clean

build: client.c server.c
	$(CC) -o client client.c
	$(CC) -o server server.c

test: server
	./server

clean:
	rm -f client server
