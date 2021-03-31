CFLAGS = -I. -lpq
CC = gcc

test: test.c makefile
	$(CC) -g -Wall -o test test.c $(CFLAGS)

clean :
	rm test