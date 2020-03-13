CC=gcc -Wall

all: build test clean

build: main.c
	$(CC) -o main main.c

test: main
	mkdir tests
	@echo '**** 20000 records with size of 1 byte ****'
	@./main generate tests/t1 20000 1
	@echo copy using system functions
	@./main copy tests/t1 tests/t1_1 20000 1 sys
	@echo copy using library functions
	@./main copy tests/t1 tests/t1_2 20000 1 lib
	@echo sort using system functions
	@./main sort tests/t1_1 20000 1 sys
	@echo sort using library functions
	@./main sort tests/t1_2 20000 1 lib
	@echo '**** 20000 records with size of 1 byte ****'
	@./main generate tests/t2 30000 1
	@echo copy using system functions
	@./main copy tests/t2 tests/t2_1 30000 1 sys
	@echo copy using library functions
	@./main copy tests/t2 tests/t2_2 30000 1 lib
	@echo sort using system functions
	@./main sort tests/t2_1 30000 1 sys
	@echo sort using library functions
	@./main sort tests/t2_2 30000 1 lib
	@echo '**** 50000 records with size of 4 bytes ****'
	@./main generate tests/t3 50000 4
	@echo copy using system functions
	@./main copy tests/t3 tests/t3_1 50000 4 sys
	@echo copy using library functions
	@./main copy tests/t3 tests/t3_2 50000 4 lib
	@echo sort using system functions
	@./main sort tests/t3_1 50000 4 sys
	@echo sort using library functions
	@./main sort tests/t3_2 50000 4 lib
	@echo '**** 100000 records with size of 4 bytes ****'
	@./main generate tests/t4 100000 4
	@echo copy using system functions
	@./main copy tests/t4 tests/t4_1 100000 4 sys
	@echo copy using library functions
	@./main copy tests/t4 tests/t4_2 100000 4 lib
	@echo sort using system functions
	@./main sort tests/t4_1 100000 4 sys
	@echo sort using library functions
	@./main sort tests/t4_2 100000 4 lib
	@echo '**** 50000 records with size of 512 bytes ****'
	@./main generate tests/t5 50000 512
	@echo copy using system functions
	@./main copy tests/t5 tests/t5_1 50000 512 sys
	@echo copy using library functions
	@./main copy tests/t5 tests/t5_2 50000 512 lib
	@echo sort using system functions
	@./main sort tests/t5_1 50000 512 sys
	@echo sort using library functions
	@./main sort tests/t5_2 50000 512 lib
	@echo '**** 100000 records with size of 512 bytes ****'
	@./main generate tests/t6 100000 512
	@echo copy using system functions
	@./main copy tests/t6 tests/t6_1 100000 512 sys
	@echo copy using library functions
	@./main copy tests/t6 tests/t6_2 100000 512 lib
	@echo sort using system functions
	@./main sort tests/t6_1 100000 512 sys
	@echo sort using library functions
	@./main sort tests/t6_2 100000 512 lib
	@echo '**** 50000 records with size of 1024 bytes ****'
	@./main generate tests/t7 50000 1024
	@echo copy using system functions
	@./main copy tests/t7 tests/t7_1 50000 1024 sys
	@echo copy using library functions
	@./main copy tests/t7 tests/t7_2 50000 1024 lib
	@echo sort using system functions
	@./main sort tests/t7_1 50000 1024 sys
	@echo sort using library functions
	@./main sort tests/t7_2 50000 1024 lib
	@echo '**** 100000 records with size of 1024 bytes ****'
	@./main generate tests/t8 100000 1024
	@echo copy using system functions
	@./main copy tests/t8 tests/t8_1 100000 1024 sys
	@echo copy using library functions
	@./main copy tests/t8 tests/t8_2 100000 1024 lib
	@echo sort using system functions
	@./main sort tests/t8_1 100000 1024 sys
	@echo sort using library functions
	@./main sort tests/t8_2 100000 1024 lib
	@echo '**** 50000 records with size of 4096 bytes ****'
	@./main generate tests/t9 50000 4096
	@echo copy using system functions
	@./main copy tests/t9 tests/t9_1 50000 4096 sys
	@echo copy using library functions
	@./main copy tests/t9 tests/t9_2 50000 4096 lib
	@echo sort using system functions
	@./main sort tests/t9_1 50000 4096 sys
	@echo sort using library functions
	@./main sort tests/t9_2 50000 4096 lib
	@echo '**** 100000 records with size of 4096 bytes ****'
	@./main generate tests/t10 100000 4096
	@echo copy using system functions
	@./main copy tests/t10 tests/t10_1 100000 4096 sys
	@echo copy using library functions
	@./main copy tests/t10 tests/t10_2 100000 4096 lib
	@echo sort using system functions
	@./main sort tests/t10_1 100000 4096 sys
	@echo sort using library functions
	@./main sort tests/t10_2 100000 4096 lib
	@echo '**** 50000 records with size of 8192 bytes ****'
	@./main generate tests/t11 50000 8192
	@echo copy using system functions
	@./main copy tests/t11 tests/t11_1 50000 8192 sys
	@echo copy using library functions
	@./main copy tests/t11 tests/t11_2 50000 8192 lib
	@echo sort using system functions
	@./main sort tests/t11_1 50000 8192 sys
	@echo sort using library functions
	@./main sort tests/t11_2 50000 8192 lib
	@echo '**** 100000 records with size of 8192 bytes ****'
	@./main generate tests/t12 100000 8192
	@echo copy using system functions
	@./main copy tests/t12 tests/t12_1 100000 8192 sys
	@echo copy using library functions
	@./main copy tests/t12 tests/t12_2 100000 8192 lib
	@echo sort using system functions
	@./main sort tests/t12_1 100000 8192 sys
	@echo sort using library functions
	@./main sort tests/t12_2 100000 8192 lib

clean:
	rm -rf main tests
