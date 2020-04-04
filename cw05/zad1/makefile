CC=gcc -Wall

all: build test clean

build: main.c
	$(CC) -o main main.c

test: main commands
	./main commands

clean:
	rm -f main
