CC=gcc -Wall

all: build example_test clean

build: main.c
	$(CC) -o main main.c -lpthread -lm

example_test: main
	./main 4 sign roi_14.ascii.pgm output_sign
	./main 4 block roi_14.ascii.pgm output_block
	./main 4 interleaved roi_14.ascii.pgm output_interleaved

full_test: main
	./main 1 sign roi_14.ascii.pgm output_sign
	./main 1 block roi_14.ascii.pgm output_block
	./main 1 interleaved roi_14.ascii.pgm output_interleaved
	./main 2 sign roi_14.ascii.pgm output_sign
	./main 2 block roi_14.ascii.pgm output_block
	./main 2 interleaved roi_14.ascii.pgm output_interleaved
	./main 4 sign roi_14.ascii.pgm output_sign
	./main 4 block roi_14.ascii.pgm output_block
	./main 4 interleaved roi_14.ascii.pgm output_interleaved
	./main 8 sign roi_14.ascii.pgm output_sign
	./main 8 block roi_14.ascii.pgm output_block
	./main 8 interleaved roi_14.ascii.pgm output_interleaved

clean:
	rm -f main output_sign output_block output_interleaved
