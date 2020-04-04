CC=gcc -Wall

all: build test clean

build: consumer.c producer.c main.c
	$(CC) -o consumer consumer.c
	$(CC) -o producer producer.c
	$(CC) -o main main.c

test: consumer producer main
	./main

clean:
	rm -f producer consumer main pipe

clean_data:
	rm -f input* output
