CC = gcc
CFLAGS = -Wall -g -std=c99

build: quadtree.o
	$(CC) -o quadtree $^

quadtree.o: quadtree.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f *.o quadtree

.PHONY: clean build
