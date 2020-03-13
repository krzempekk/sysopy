CC=gcc -Wall

all: build test clean

build: main.c
	$(CC) -o main main.c

test: main
	mkdir d1
	touch d1/f1
	touch d1/f2
	mkdir d1/d2
	touch d1/d2/f1
	touch d1/d2/f2
	ln -s d1/f2 d1/d2/l1
	mkdir d1/d3
	touch d1/d3/f1
	ln -s d1/d2/f1 d1/d3/l1
	./main d1

clean:
	rm -f main
	rm -rf d1