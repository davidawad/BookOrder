CC = gcc
CFLAGS = -Wall -g

all: main

main:
	$(CC) $(CFLAGS) -o bookOrder main.c -lpthread

run: main
	./bookOrder database.txt orders.txt categories.txt

clean:
	rm -f bookOrder
	rm -f *.o
