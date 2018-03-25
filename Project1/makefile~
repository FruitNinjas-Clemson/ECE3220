# Austin Johnson 
# CPSC 3220  
# Project 1 
# Makefile

CFLAGS = -Wall -g

all: leakcount.o memory_shim.so sctracer.o

leakcount.o: leakcount.c
	gcc $(CFLAGS) -o leakcount leakcount.c

memory_shim.so: memory_shim.c
	gcc $(CFLAGS) -shared -fPIC -o memory_shim.so memory_shim.c -ldl

sctracer.o: sctracer.c
	gcc $(CFLAGS) -o sctracer sctracer.c

clean :
	rm leakcount.o memory_shim.so sctracer.o
