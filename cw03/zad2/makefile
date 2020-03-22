CC=gcc -Wall

all: build generate_tests run_tests check_tests clean

build: generate.c main.c test.c
	$(CC) -o generate generate.c
	$(CC) -o main main.c -lm
	$(CC) -o test test.c

generate_tests: generate
	mkdir tests
	./generate 10 5 50

run_tests: main
	./main tests_list 4 1 1

check_tests: test
	./test tests_list

clean:
	rm -rf tests generate main test tests_list