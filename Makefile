CC = gcc
CFLAGS = -Wall -g

all: main

main: clean
	$(CC) $(CFLAGS) -o bookOrder bookOrder.c -lpthread

run: main
	./bookOrder database.txt orders.txt categories.txt

time: main
	time ./bookOrder database.txt orders.txt categories.txt

clean:
	clear
	rm -f bookOrder
	rm -f *.o
