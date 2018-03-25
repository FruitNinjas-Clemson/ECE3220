# Austin Johnson
# CPSC 3220
# Project 2
# Makefile

CFLAGS = -Wall -g -c

all: list.o mythreads.o libmythreads.a 

list.o: list.c
	gcc $(CFLAGS) list.c

mythreads.o: mythreads.c
	gcc $(CFLAGS) mythreads.c

libmythreads.a:
	ar -cvr libmythreads.a list.o mythreads.o

clean :
	rm list.o mythreads.o libmythreads.a
