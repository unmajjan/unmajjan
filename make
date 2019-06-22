CC = gcc
CFLAGS = -lpng -lX11 -lm

all :
	$(CC) -Wall -Werror -Wuninitialized unmj_Demo.c unmajjan.c $(CFLAGS)  -o unmj_Demo
	./unmj_Demo
