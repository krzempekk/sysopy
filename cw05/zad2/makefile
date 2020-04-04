CC=gcc -Wall

all: build test clean

build: main.c
	$(CC) -o main main.c

test: main file
	./main file

clean:
	rm -f main
