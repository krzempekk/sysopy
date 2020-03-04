CC=gcc

all: static shared clean

static:
	$(CC) -c library.c
	ar rcs library.a library.o

shared:
	$(CC) -fPIC -c library.c
	$(CC) -fPIC -shared -o library.so library.o

clean:
	rm -f *.o