CC=gcc -Wall

all: build test clean

build: client.c server.c
	$(CC) -o client client.c common.c -lpthread
	$(CC) -o server server.c common.c -lpthread

test: client server
	./server

clean:
	rm -f client server sock
