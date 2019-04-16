CFLAGS = -Wall -Wextra -std=c11 -O2
CC = gcc

all: main
node.o: node.c node.h
	$(CC) $(CFLAGS) -c node.c
list.o: list.c list.h node.h
	$(CC) $(CFLAGS) -c list.c
user.o: user.c user.h pair.h list.h node.h
	$(CC) $(CFLAGS) -c user.c
main.o: main.c user.h
	$(CC) $(CFLAGS) -c main.c
main: main.o user.o list.o node.o
	$(CC) -o main main.o user.o list.o node.o
clean:
	-rm node.o list.o user.o main.o main
