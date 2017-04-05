CC = gcc
CFLAGS = -Wall -Wextra -lpng -lm

all : flots_main

flots_main : utils_png.c
	$(CC) utils_png.c -o flots_main $(CFLAGS)

clean :
	rm -f flots_main test.png
