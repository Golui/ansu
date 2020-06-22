all: microtar

microtar:
	mkdir -p bin
	$(C) -MMD -MP -Wall -g -pedantic -o bin/microtar.o -Iinclude -c src/microtar.c
	ar rcs libmicrotar.a bin/microtar.o
