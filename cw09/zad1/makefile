CC=gcc -Wall

all: build test clean

build: main.c
	$(CC) -o main main.c -lpthread -lm

test: main
	./main 2 10

clean:
	rm -f main
