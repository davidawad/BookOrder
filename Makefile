CC = gcc
CFLAGS = -Wall -g

all: main

main: clean
	$(CC) $(CFLAGS) -o bookOrder main.c -lpthread

run: clean main
	./bookOrder database.txt orders.txt categories.txt

time: main
	time ./bookOrder database.txt orders.txt categories.txt #> cpuTimeLog.txt
	##cat cpuTimeLog.txt

clean:
	rm -f bookOrder
	rm -f *.o
