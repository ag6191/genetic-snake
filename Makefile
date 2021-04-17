# Snake Engine Timer (SET) Makefile
CC = gcc
CFLAGS = -Iinclude -Wall -g
FILE = parameters
OBJS = obj/main.o obj/gsdriver.o obj/gsthread.o obj/gsprint.o obj/envcntr.o obj/nncntr.o obj/nnfuncts.o obj/envfuncts.o obj/gsutils.o


all: build run

build: clean
	mkdir obj
	$(CC) $(CFLAGS) -c -o obj/gsdriver.o src/gs/gsdriver.c
	$(CC) $(CFLAGS) -c -o obj/gsutils.o src/gs/gsutils.c
	$(CC) $(CFLAGS) -c -o obj/gsthread.o src/gs/gsthread.c
	$(CC) $(CFLAGS) -c -o obj/nnfuncts.o src/nn/nnfuncts.c
	$(CC) $(CFLAGS) -c -o obj/nncntr.o src/nn/nncntr.c
	$(CC) $(CFLAGS) -c -o obj/envfuncts.o src/env/envfuncts.c
	$(CC) $(CFLAGS) -c -o obj/envcntr.o src/env/envcntr.c
	$(CC) $(CFLAGS) -c -o obj/gsprint.o src/gs/gsprint.c
	$(CC) $(CFLAGS) -c -o obj/main.o src/main.c
	mkdir bin
	$(CC) $(CFLAGS) -o bin/main $(OBJS) 

run: 
	bin/main $(FILE)

clean: 
	-rm -rf bin obj


