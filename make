CC = gcc
CFLAGS = -lpng -lX11 -lm

all :
	$(CC) unmj_Demo.c unmajjan.c $(CFLAGS)  -o unmj_Demo
	./unmj_Demo
