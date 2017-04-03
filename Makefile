CC = gcc
CFLAGS = -Wall -Wextra -lpng -lm

all : utils_png

utils_png : utils_png.c
	$(CC) utils_png.c -o $@ $(CFLAGS)

clean :
	rm -f utils_png test.png
