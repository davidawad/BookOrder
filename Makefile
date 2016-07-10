CC = gcc
CFLAGS = -Wall -g -pthread

all: main

main: clean
	$(CC) $(CFLAGS)  bookOrder.c cust.c order.c util.c category.c -o bookOrder
run: main
	./bookOrder database.txt orders.txt categories.txt

time: main
	time ./bookOrder database.txt orders.txt categories.txt

clean:
	clear
	rm -f bookOrder
	rm -f *.o
