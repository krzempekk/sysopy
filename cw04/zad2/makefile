CC=gcc -Wall

all: build

build: main.c prog.c
	$(CC) -o main main.c
	$(CC) -o prog prog.c

clean: main prog
	rm main prog
