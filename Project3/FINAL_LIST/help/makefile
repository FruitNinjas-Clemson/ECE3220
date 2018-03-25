# Austin Johnson
# CPSC 3220
# Project 3
# Makefile

CFLAGS = -Wall -g

all: libmyalloc.so

libmyalloc.so:
		gcc $(CFLAGS) -shared -fPIC -o libmyalloc.so list.c allocator.c -lm -ldl

clean :
	rm libmyalloc.so
